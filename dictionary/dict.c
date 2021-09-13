/* STD IO */
#include <stdio.h>
#include <string.h> /* Needed for strerror() */

/* Shared Memory/Semaphore */
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

/* Errors */
#include <errno.h>
extern int errno;	/* Global error number */

/* Defines to make code easier to read */
#define TRUE			(1)
#define FALSE			(0)
#define SUCCESS			(TRUE)
#define FAILURE			(FALSE)

#define DEBUG			(FALSE)		/* Print debug statements? */
#define LIST_ENDPOINT		(-1)		/* Linked list endpoint */

/* Dictionary config */
#define MAX_WORDS		(1000000)	/* Max words that can be stored */
#define MAX_WORD_LEN		(64)		/* Max length of a single word */

/* Shared memory/Semaphore config */
#define SHRMEM_KEY		(2)		/* Shared memory key ID*/
#define SHRMEM_PRIV_RO		(0444)		/* Shared memory read-only */
#define SHRMEM_PRIV_RW		(0666)		/* Shared memory read-write */

#define SEM_KEYID		(2)		/* Semaphore key ID */
#define SEM_PRIV_RW		(0666)		/* Semaphore read-write */
#define SEM_NUM			(1)		/* Number of semaphores */

/* Command-line options */
#define OPT_NUMREQ		(3)		/* # of required command line arguments required */
#define OPT_STRLEN		(6)		/* String length of command line arguments */

#define OPT_SEARCH_STR		("search")	/* Search for the existence of a word in the dictionary */
#define OPT_INSERT_STR		("insert")	/* Insert word into the dictionary */
#define OPT_DELETE_STR		("delete")	/* Delete word from the dictionary */
#define USAGE_STR       	("\nUsage:  dict {insert|search|delete} <word>\n\n")

/* Integer representations of command-line options */ 
typedef enum
{
    OPT_SEARCH_NUM = 1,
    OPT_INSERT_NUM,
    OPT_DELETE_NUM
}
DICT_OPT;

/* Define a Word (Node) in the list, also used as a Header record */
typedef struct
{
	int	used_cnt;			/* If Word:   1=this memory is in use, 0=free
						   If Header: This stores the total count of words	*/ 

	char	word[MAX_WORD_LEN + 1];		/* The word */

	int	slot_prev;			/* If Word:   The word before this is in slot N
					 	   If Header: The start of the list */

	int	slot;				/* This word is in slot N */

	int	slot_next;			/* If Word:   The word after this is in slot N
	                                           If Header: The next open memory slot */ 
}
WORD;

/* Define the size of shared memory, provide space for a Header and MAX_WORDS words */
#define SHRMEM_SIZE ((1 + MAX_WORDS) * sizeof(WORD))


/* Function prototypes */
void  report_error(char *errstr, int errnum);

void *attach_shrmem(char *prog_path, int shrmem_priv);
int   detach_shrmem(char *prog_path, void *shrmem_p);

WORD *get_open_slot(WORD *header_p, WORD *slots_p);
char *prep_word_str(char *strbuf);
WORD *find_list_start(WORD *header_p, WORD *slots_p);
WORD *traverse_list(WORD *slots_p, WORD *cur_p, int direction);
WORD *find_word_node(int list_size, WORD *slots_p, WORD *cur_p, char *word_str);

int   search_word(WORD *header_p, WORD *slots_p, char *word_str);
int   insert_word(WORD *header_p, WORD *slots_p, char *word_str);
int   delete_word(WORD *header_p, WORD *slots_p, char *word_str);


int main(int argc, char *argv[])
{
	void 	       *shrmem_p      = NULL;
	WORD	       *dict_p        = NULL;
	WORD	       *header_p      = NULL; 	
	WORD	       *slots_p       = NULL;
	DICT_OPT	operation_num = 0;
	char		word[MAX_WORD_LEN + 1];

	memset(word, 0x00, sizeof(word));

	/* Verify/Get command-line options */
	if( argc != OPT_NUMREQ )
	{
		report_error("Incorrect options", 0);
		printf(USAGE_STR);
		return (1);
	}

	/* Get operation to perform */
	if( !strncmp(OPT_SEARCH_STR, argv[1], OPT_STRLEN) )
		operation_num = OPT_SEARCH_NUM;
	else if( !strncmp(OPT_INSERT_STR, argv[1], OPT_STRLEN) )
		operation_num = OPT_INSERT_NUM;
	else if( !strncmp(OPT_DELETE_STR, argv[1], OPT_STRLEN) )
		operation_num = OPT_DELETE_NUM;
	if( operation_num == 0 )
	{
		report_error("Invalid operation", 0);
		printf(USAGE_STR);
		return (1);
	}
	
	/* Get Word */
	if( strlen(argv[2]) > MAX_WORD_LEN )
	{
		report_error("Word is too long", 0);
		printf(USAGE_STR);
		return (1);
	}
	snprintf(word, sizeof(word), "%s", argv[2]);
	
	/* Convert word to lowercase and remove any non-printable chars,
	   then check length again */	
	prep_word_str(word);
	if( (strlen(word) > MAX_WORD_LEN) || (strlen(word) <= 0) )
	{
		report_error("Invalid word length or non-printable characters", 0);
		printf(USAGE_STR);
		return (1);
	}

	/* Attach to shared memory */
	shrmem_p = attach_shrmem(argv[0], SHRMEM_PRIV_RW);
	if( !shrmem_p )
		return (1); /* Error was already reported */

	/* Get pointers to dictionary */
	dict_p   = (WORD *) shrmem_p;
	header_p = &dict_p[0];	/* Header record */
	slots_p  = &dict_p[1];	/* Word memory slots */
	if( DEBUG ) printf("TOTAL WORDS: %d\n", header_p->used_cnt);

	/* Perform requested operation */
	switch( operation_num )
	{
		case OPT_SEARCH_NUM:
			search_word(header_p, slots_p, word);
			break;

		case OPT_INSERT_NUM:
			insert_word(header_p, slots_p, word);
			break;

		case OPT_DELETE_NUM:
			delete_word(header_p, slots_p, word);
			break;
		
		default:
			break;
	}

	/* Detach from shared memory */
	if( detach_shrmem(argv[0], shrmem_p) != SUCCESS )
		return (1);

	return 0;
}


/* Report error to console
   Input:
   errstr - Error string buffer
   errnum - Error number (errno)
   
   Returns:
   (none)
*/
void report_error(char *errstr, int errnum)
{
	if( errnum )
		printf("%s Error: (%d) %s\n", errstr, errnum, strerror(errnum));
	else
		printf("Error: %s\n", errstr);
}


/* Create/Check Semaphore and attach to shared memory
   Input:
   prog_path   - Program path and name (used for shm/sem keys)
   shrmem_priv - Shared memory privileges
   
   Returns:
   Pointer to shared memory, NULL on failure
*/
void *attach_shrmem(char *prog_path, int shrmem_priv)
{
	const size_t	shm_size = SHRMEM_SIZE;

	int		init      = FALSE;	/* Initialize shared memory? (first run) */

	key_t		sem_key    = (-1);	/* Semaphore key */
	int		sem_id     = (-1);
	struct sembuf	sem_oper[2];

	key_t		shrmem_key = (-1);	/* Shared memory key */
	int		shrmem_id  = (-1);	/* Shared memory ID */
	void	       *shrmem_p   = NULL;	/* Pointer to shared memory */
	int		shmat_priv = 0;		/* Attached shared memory privileges */

	memset(sem_oper, 0x00, sizeof(sem_oper));
	errno = 0;


	/* Try to create semaphore */
	sem_key = ftok(prog_path, SEM_KEYID);
	sem_id  = semget(sem_key, SEM_NUM, IPC_CREAT | IPC_EXCL | SEM_PRIV_RW);
	if( (sem_id < 0) || errno )
	{
		/* Create Failed */
		if( EEXIST == errno )
		{
			/* Already exists, get ID */
			errno  = 0;
			sem_id = semget(sem_key, SEM_NUM, SEM_PRIV_RW);
			if( (sem_id < 0) || errno )
			{
				/* Report error and fail */
				report_error("SEMGET", errno);
				return NULL;
			}
		}		
		else
		{
			/* Report error and fail */
			report_error("SEMGET/CREATE", errno);
			return NULL;
		}
	}
	errno = 0;


	/* Try to create shared memory section (create with RW priv) */
	shrmem_key = ftok(prog_path, SHRMEM_KEY);
	shrmem_id  = shmget(shrmem_key, shm_size, IPC_CREAT | IPC_EXCL | SHRMEM_PRIV_RW);
	if( (shrmem_id < 0) || errno )
	{
		/* Create Failed */
		if( EEXIST == errno )
		{
			/* Already exists, get ID */
			errno = 0;
			shrmem_id = shmget(shrmem_key, shm_size, shrmem_priv);
			if( (shrmem_id < 0) || errno )
			{
				/* Report error and fail */
				report_error("SHMGET", errno);
				return NULL;
			}
		}		
		else
		{
			/* Report error and fail */
			report_error("SHMGET/CREATE", errno);
			return NULL;
		}
	}
	else
	{
		/* Create succeeded, initialize after attach */
		init = TRUE;
	}
	errno = 0;


	/* Define/perform semaphore operations */
	sem_oper[0].sem_num = 0;	/* Operate on sem 0 */
	sem_oper[0].sem_op  = 0;	/* Wait for a value of 0 */
	sem_oper[0].sem_flg = 0;	/* Allow wait */

	sem_oper[1].sem_num = 0;	/* Operate on sem 0 */
	sem_oper[1].sem_op  = 1;	/* Increment by 1 */
	sem_oper[1].sem_flg = 0;	/* Allow wait */

	semop(sem_id, sem_oper, 2);
	if( errno )
	{
		/* Report error and fail */
		report_error("SEMOP", errno);
		return NULL;
	}
	errno = 0;
	if( DEBUG) printf("Incremented semaphore, ID: %d\n", sem_id);


	/* Attach to shared memory */
	if( SHRMEM_PRIV_RO == shrmem_priv)
		shmat_priv = SHM_RDONLY;

	shrmem_p = shmat(shrmem_id, NULL, shmat_priv);
	if( !shrmem_p || errno )
	{
		/* Report error and fail */
		report_error("SHMAT", errno);
		return NULL;
	}
	if( DEBUG) printf("Attached to shared memory, ID: %d\n", shrmem_id);


	/* Initialize shared memory if we just created it */	
	if( init )
	{
		memset(shrmem_p, 0x00, shm_size);
		if( DEBUG) printf("Initialized shared memory (size %d)\n", (int) shm_size);
	}

	return shrmem_p;
}


/* Detach from shared memory and decrement semaphore
   Input:
   prog_path - Program path and name (used for sem key)
   shrmem_p  - Pointer to shared memory
   
   Returns:
   SUCCESS or FAILURE
*/
int detach_shrmem(char *prog_path, void *shrmem_p)
{
	key_t		sem_key = ftok(prog_path, SEM_KEYID);
	int		sem_id  = (-1);
	struct sembuf	sem_oper[1];

	memset(sem_oper, 0x00, sizeof(sem_oper));
	errno = 0;

	/* Detach shared memory */
	shmdt(shrmem_p);
	if( errno )
	{
		/* Report error and fail */
		report_error("SHMDT", errno);
		return FAILURE;
	}
	if( DEBUG ) printf("Detached from shared memory\n");


	/* Get semaphore ID */
	sem_id = semget(sem_key, SEM_NUM, SEM_PRIV_RW);
	if( (sem_id < 0) || errno )
	{
		/* Report error and fail */
		report_error("SEMGET", errno);
		return FAILURE;
	}

	/* Define/perform semaphore operations */
	sem_oper[0].sem_num = 0;		/* Operate on sem 0 */
	sem_oper[0].sem_op  = (-1);	/* Decrement semaphore value */
	sem_oper[0].sem_flg = 0;		/* Allow wait */
	semop(sem_id, sem_oper, 1);
	if( errno )
	{
		/* Report error and fail */
		report_error("SEMOP", errno);
		return FAILURE;
	}
	if( DEBUG ) printf("Decremented semaphore\n");

	return SUCCESS;
}


/* Convert a string to lowercase and filter out non-printable chars
   Input/Output
   strbuf - String buffer (assumed to be sized at (MAX_WORD_LEN+1) max
   
   Returns:
   (char *) pointer to string buffer
*/
char* prep_word_str(char *strbuf)
{
	int	i, j;
	char	tmpstr[MAX_WORD_LEN + 1];

	/* Save the string to local buffer */
	memset(tmpstr, 0x00, sizeof(tmpstr));
	snprintf(tmpstr, sizeof(tmpstr), "%s", strbuf);

	/* Rebuild the string, remove non-printable chars and convert to lowercase */
	for( i = 0, j = 0; (i <= strlen(tmpstr)) && (i <= MAX_WORD_LEN) && (j <= MAX_WORD_LEN); i++ )
	{
		if( isprint(tmpstr[i]) )
			strbuf[j++] = tolower(tmpstr[i]);
	}
	strbuf[j] = '\0'; /* Ensure null terminator present */
	
	return strbuf;
}


/* Get pointer to an open slot in the word memory
   Input:
   header_p - Pointer to dictionary header
   slots_p  - Pointer to start of word memory slots
   
   Returns:
   SUCCESS or FAILURE
*/
WORD *get_open_slot(WORD *header_p, WORD *slots_p)
{
	int	i; /* Counter */
	int	open_slot = LIST_ENDPOINT;
	WORD   *slot_p    = NULL;

	/* Try finding an open slot (starting at first likely open slot) */
	for(i = header_p->slot_next; i < MAX_WORDS; i++)
	{
		slot_p = &slots_p[i];
		if( FALSE == slot_p->used_cnt )
		{
			open_slot = i;
			break;
		}
	}

	/* Try finding an open slot (starting back at beginning) */
	if( open_slot == LIST_ENDPOINT )
	{
		for(i = 0; i < MAX_WORDS; i++)
		{
			slot_p = &slots_p[i];
			if( FALSE == slot_p->used_cnt )
			{
				open_slot = i;
				break;
			}
		}
	}

	if( open_slot != LIST_ENDPOINT )
	{
		/* Open slot found */
		memset(slot_p, 0x00, sizeof(WORD));
		slot_p->slot_prev = LIST_ENDPOINT;
		slot_p->slot      = open_slot;
		slot_p->slot_next = LIST_ENDPOINT;

		if( DEBUG ) printf("Found free slot, IDX: %d\n", open_slot);
		return slot_p;
	}
	
	/* No open slot found */
	report_error("No open memory slot found", 0);	
	return NULL;
}


/* Find the start of the list
   Input:
   header_p - Pointer to dictionary header
   slots_p  - Pointer to start of word memory slots
   
   Returns:
   (WORD *) - Pointer to the first word in the list
*/
WORD *find_list_start(WORD *header_p, WORD *slots_p)
{
	int		i;
	WORD	       *list_p = NULL;			  
	
	/* Try finding start of list (starting at known first word) */
	for(i = header_p->slot_prev; i < MAX_WORDS; i++)
	{
		list_p = &slots_p[i];
		if( list_p->used_cnt && (LIST_ENDPOINT == list_p->slot_prev) )
		{
			header_p->slot_prev = i;
			return list_p;
		}
	}

	/* Try finding start of list (starting back at beginning) */
	for(i = 0; i < MAX_WORDS; i++)
	{
		list_p = &slots_p[i];
		if( list_p->used_cnt && (LIST_ENDPOINT == list_p->slot_prev) )
		{
			header_p->slot_prev = i;
			return list_p;
		}
	}
	
	return NULL; /* List is empty */	
}


/* Given a node, traverse the list 1 word in a specified direction
   Input:
   slots_p   - Pointer to start of word memory slots
   cur_p     - Pointer to the word to start from
   direction - Direction to traverse (<0=left, >0=right, 0=stay)
   
   Returns:
   (WORD *) - New word reached (NULL if none)
*/
WORD *traverse_list(WORD *slots_p, WORD *cur_p, int direction)
{
	if( !cur_p )
		return NULL; /* Null pointer passed */

	if( direction == 0 )
	{
		/* Stay here (return) */
		return cur_p;
	}
	if( (direction < 0) && (LIST_ENDPOINT != cur_p->slot_prev) )
	{
		/* Go left 1 */
		return &slots_p[cur_p->slot_prev];
	}
	if( (direction > 0) && (LIST_ENDPOINT != cur_p->slot_next) )
	{
		/* Go right 1 */
		return &slots_p[cur_p->slot_next];
	}

	return NULL; /* Can't traverse list */ 
}


/* Search the dictionary for a word's node
   Input:
   list_size - The number of words in the list 
   slots_p   - Pointer to start of word memory slots
   cur_p     - Pointer to the word to start from
   word_str  - Word to search for
   
   Returns:
   (WORD *) - Pointer to search word or closest word to it
*/
WORD *find_word_node(int list_size, WORD *slots_p, WORD *cur_p, char *word_str)
{
	int		cmp_result;
	WORD	       *right_p;
	WORD	       *left_p;

	if( !cur_p )
		return NULL;	/* Null node passed */

	if( list_size <= 1 )
		return cur_p;	/* List too small to search */

	cmp_result = strcmp(cur_p->word, word_str);

	/* Is this the search word? */
	if( cmp_result == 0 )
		return cur_p; /* Found word! */

	/* Get the words to the left and right.
	   If the search word is between them its not in the list,
	   just return the current word. */
	right_p = traverse_list(slots_p, cur_p, 1);
	left_p  = traverse_list(slots_p, cur_p, -1);
	if(   (!right_p || (strcmp(right_p->word, word_str) > 0))
	   && (!left_p || (strcmp(left_p->word, word_str) < 0)) )
	{
		return cur_p; /* Word isn't in list */
	}

	/* Keep searching */
	if( cmp_result < 0 )
	{
		/* Word is to the right */
		return find_word_node(list_size, slots_p, right_p, word_str);
	}
	else if( cmp_result > 0 )
	{
		/* Word is to the left */
		return find_word_node(list_size, slots_p, left_p, word_str);
	}

	return NULL; /* Shouldn't get here */
}


/* Search the dictionary for the existence of a word
   Input:
   header_p - Pointer to dictionary header
   slots_p  - Pointer to start of word memory slots
   word_str - Word to search for
   
   Returns:
   TRUE=Word found, FALSE=Word not found
*/
int search_word(WORD *header_p, WORD *slots_p, char *word_str)
{
	WORD			  *list_p;
	WORD			  *word_p;

	/* Get the start of the list (null if empty) */
	list_p = find_list_start(header_p, slots_p);

	/* Find the word in the list */
	word_p = find_word_node(header_p->used_cnt, slots_p, list_p, word_str);
	if( word_p && (strcmp(word_str, word_p->word) == 0) )
	{
		printf("%s - exists in dictionary\n", word_str);
		return TRUE;
	}

	printf("%s - does not exist in dictionary\n", word_str);
	return FALSE;
}


/* Insert a word into the dictionary
   Input:
   header_p - Pointer to dictionary header
   slots_p  - Pointer to start of word memory slots
   word_str - Word to insert
   
   Returns:
   TRUE=Word added, FALSE=Word not added
*/
int insert_word(WORD *header_p, WORD *slots_p, char *word_str)
{
	int			cmp_result;
	WORD		       *list_p;
	WORD		       *temp_word_p;
	WORD		       *prev_word_p;
	WORD		       *this_word_p;
	WORD		       *next_word_p;

	/* Check if the dictionary is full already */
	if( header_p->used_cnt >= MAX_WORDS )
	{
		report_error("Dictionary full", 0);
		return FALSE;
	}

	/* Add new word to dictionary (unless a duplicate) */
	if( header_p->used_cnt == 0 )
	{
		/* This will be added at the front of the list as the first word */
		this_word_p = get_open_slot(header_p, slots_p);
		if( !this_word_p )
			return FALSE; /* No open slots, error was already reported */
	}
	else
	{
		/* Get the start of the list (null if empty) */
		list_p = find_list_start(header_p, slots_p);

		/* Find word or closest word */
		temp_word_p = find_word_node(header_p->used_cnt, slots_p, list_p, word_str);
		if( !temp_word_p )
		{
			report_error("Error getting reference word", 0);
			return FALSE;
		}
		
		/* Compare word to found */ 
		cmp_result = strcmp(temp_word_p->word, word_str);
		if( cmp_result == 0 )
		{
			printf("%s - already in dictionary\n", word_str);
			return FALSE;
		}
		else if( cmp_result > 0 )
		{
			/* Word should be placed left of reference word */
			prev_word_p = traverse_list(slots_p, temp_word_p, -1);
			next_word_p = temp_word_p;
		}
		else if( cmp_result < 0 )
		{
			/* Word should be placed right of reference word */
			prev_word_p = temp_word_p;
			next_word_p = traverse_list(slots_p, temp_word_p, 1);
		}

		/* Get an open memory slot */		
		this_word_p = get_open_slot(header_p, slots_p);
		if( !this_word_p )
			return FALSE; /* No open slots, error was already reported */

		/* The next slot will (likely) be open for the next run */
		header_p->slot_next = (this_word_p->slot + 1);

		/* Place the new word between the prev/next words */
		if( next_word_p )
		{
			next_word_p->slot_prev = this_word_p->slot;
			this_word_p->slot_next = next_word_p->slot;
		}
		if( prev_word_p )
		{
			prev_word_p->slot_next = this_word_p->slot;
			this_word_p->slot_prev = prev_word_p->slot;
		}
	}

	/* Populate the memory slot with the new word */
	this_word_p->used_cnt = TRUE;
	sprintf(this_word_p->word, "%s", word_str); 


	/* Is this slot now the start of the list? */
	if( LIST_ENDPOINT == this_word_p->slot_prev )
		header_p->slot_prev = this_word_p->slot; /* Save to header so the first word is known */

	/* Successfully added new word, increment Header counter and return */
	header_p->used_cnt++;
	printf("%s - inserted into dictionary\n", word_str);
	return TRUE;
}


/* Delete a word from the dictionary
   Input:
   header_p - Pointer to dictionary header
   slots_p  - Pointer to start of word memory slots
   word_str - Word to delete
   
   Returns:
   TRUE=Word deleted, FALSE=Word not deleted
*/
int delete_word(WORD *header_p, WORD *slots_p, char *word_str)
{
	WORD			  *list_p;
	WORD			  *prev_word_p;
	WORD			  *this_word_p;
	WORD			  *next_word_p;
	
	/* Check if the dictionary is empty already */
	if( header_p->used_cnt == 0 )
	{
		printf("Dictionary empty\n");
		return FALSE;
	}

	/* Get the start of the list (null if empty) */
	list_p = find_list_start(header_p, slots_p);

	/* Find the word */
	this_word_p = find_word_node(header_p->used_cnt, slots_p, list_p, word_str);
	if( !this_word_p || strcmp(this_word_p->word, word_str) )
	{
		printf("%s - not in dictionary\n", word_str);
		return FALSE;
	}

	/* Find the word(s) before/after this one */
	prev_word_p = traverse_list(slots_p, this_word_p, -1);
	next_word_p = traverse_list(slots_p, this_word_p,  1);

	/* This slot will be open the next run */
	header_p->slot_next = this_word_p->slot;

	/* Delete the word (re-init its slot) */
	memset(this_word_p, 0x00, sizeof(WORD));
	this_word_p->slot_prev = LIST_ENDPOINT;
	this_word_p->slot      = LIST_ENDPOINT;
	this_word_p->slot_next = LIST_ENDPOINT;
	
	/* Re-link the list */
	if( prev_word_p )
	{
		if( next_word_p )
			prev_word_p->slot_next = next_word_p->slot;
		else
			prev_word_p->slot_next = LIST_ENDPOINT;
	}
	if( next_word_p )
	{
		if( prev_word_p )
			next_word_p->slot_prev = prev_word_p->slot;
		else
			next_word_p->slot_prev = LIST_ENDPOINT;
	}

	/* Successfully deleted word, decrement counter and return */
	header_p->used_cnt--;
	printf("%s - deleted from dictionary\n", word_str);
	return TRUE;
}

