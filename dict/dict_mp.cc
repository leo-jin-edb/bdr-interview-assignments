/**************************************************/
/* dict - Concurrent dictionary (multiprocessing) */
/* Author : Mukesh Kr. Singh                      */
/* Date:    1st March 2022                        */
/* Comments: this program uses shared memory      */
/*   semaphore to implement concurrent dictionary */
/* Compilation: This code is compiled and tested  */
/*  on Linux using gcc version 9.3.0              */
/*  g++ -o dict dict_mp.cc                        */
/* ************************************************/
/****************************************************/
/****************************************************
# BDR Home Assignments

## Option 1 - Dictionary

A dictionary consists of english words stored in memory.
Maximum number of words in the dictionary is 1M.

Write a command-line program that allows concurrent (by multiple processes):

 * search for existence of a word in the dictionary
 * insert a word in the dictionary, if it does not exist
 * delete a word from the dictionary

The command should be invoked as following:
```
dict {insert|search|delete} <word>
```

Languages to chose from: C or C++
Assume APIs for data structures like queue, hash table, linked list etc. are available.
Document the APIs (no implementation required) as comments.
*************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>


#include <iostream>
#include <string>


using namespace std;

#define SHM_KEY_ID 0x23456
#define SEM_KEY_ID 0x24567

#define WORDNULL  -1 // (MAX_WORDLEN + 1)       // NULL value

typedef unsigned short USHORT;
typedef unsigned char UCHAR;

const int MAX_NUM_WORD = 1000000;
const USHORT MAX_WORD_LEN = 32;
const USHORT AVG_WORD_LEN = 8;
const USHORT MAX_UNIQUE_LITERAL = 26;
const int MAX_TNODE = MAX_NUM_WORD * AVG_WORD_LEN;

// structure to store a word and its metadata

typedef struct tnode
{
    bool is_word;
    bool isddeleted;
    size_t ch[MAX_UNIQUE_LITERAL];
} tnode;

// memory pool to store words 
typedef struct WordList_tnode
{
    size_t npool;                     // used space in pool
    size_t pfree;                     // pointer to re-use freed nodes
    size_t head;                      // global list head
    tnode pool[MAX_TNODE];      // fixed-size space for nodes
}WordList_tnode;

typedef struct memorypool
{
    long max_memory_size;
    long memory_allocated;
    tnode* head;
    char tnode_pool[MAX_TNODE * sizeof(tnode)];
}memorypool;


union semun
{
    int              val;    /* Value for SETVAL */
    struct semid_ds* buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short* array;  /* Array for GETALL, SETALL */
    struct seminfo* __buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
};

WordList_tnode* pWordList;
memorypool* pshm_buf;

static int sem_id;

typedef enum op 
{
    INSERT_WORD = 1,
    SEARCH_WORD,
    DELETE_WORD
} op;



/* tnode_alloc - allocates memory to store a trie node from memory pool */
void* tnode_alloc(int size)
{

        if (pWordList->npool < MAX_TNODE)
		{
			return &pWordList->pool[pWordList->npool++];
		}
    return NULL;

};

/* getnewnode - get a new node tnode which will be assigned in child node */
tnode* getnewnode()
{
    //tnode* node = new tnode;
    tnode* node = (tnode*)tnode_alloc(sizeof (tnode));

    if (node == NULL)
    {
        perror("\n could not allocate memory for new node.Exiting ...\n");
        exit(EXIT_FAILURE);
    }
    node->is_word = false;
    for (auto i = 0; i < MAX_UNIQUE_LITERAL; i++)
        node->ch[i] = NULL;

    return node;
}

/* tnode_insertword - insert word in dictionary */
int tnode_insertword(tnode *root, const char* word)
{
    const char* ch = word;
	tnode *ptr;
    // allocate tree node point and let root node point to it 

    if (root == NULL)
    {
        root = getnewnode();
    }
    tnode* pt = root;
    while (*ch)
    {
        if (pt->ch[*ch - 'a'] == NULL)
        {
			ptr = (tnode *) getnewnode();
			
            pt->ch[*ch - 'a'] = ptr - root /*(char *) getnewnode()*/ ;
        }
        pt = root + /*(tnode *)*/ pt->ch[*ch - 'a'];
        ch++;
    }

	if (pt->is_word)
	{
		return 1;  // return 1 for duplicate word.
	}
    pt->is_word = true;

    return 0;
}
/* tnode_searchword - search word in dictionary if not found return false,else return true */
bool tnode_searchword(tnode *root, const char* word)
{
    const char* ch = word;
    tnode* pt = root;
    while (*ch)
    {
        if (pt->ch[*ch - 'a'] == NULL)
        {
            return false;
        }
        pt =  root + /*(tnode *)*/ pt->ch[*ch - 'a'];
        ch++;
    }

    return pt->is_word ? true : false;
}

tnode* tnode_searchword_del_helper(tnode *root, const char* word, USHORT* wordcount)
{
    const char* ch = word;
    tnode* pt = root;
    USHORT prefixword_length = 0;
    while (*ch)
    {
        if (pt->ch[*ch - 'a'] == NULL)
        {
            return NULL;
        }
        prefixword_length++;
        if (pt->is_word)
        {
            *wordcount = prefixword_length;
        }

        pt =  root  + /*(tnode*)*/ pt->ch[*ch - 'a'];
        ch++;
    }
    return pt->is_word ? pt : NULL;
}

void helper_delete()
{

}

bool tnode_deleteword(tnode *root, const char* word)
{
    // delete word: first serach word  and if  word not found return false.
    // if word found check if it has any child node i.e this word is prefix for some word 
    // in that case just mark this word as false
    // if word found is the last word  and has some word which is true and its prefix 
    // delete the word till that part
    // if it is the only word then delete the pointer upward.
    USHORT word_count = 0;

    tnode* pt = tnode_searchword_del_helper(root, word, &word_count);
    if (pt)
    {
        pt->is_word = false;
        return true;
    }

    /*
    if (pt)
    {
        for (int i = 0; i < MAX_UNIQUE_LITERAL; i++)
        {
            if (pt->ch[i])
            {
                pt->is_word = false;
                return true;
            }

        }

    }

    if (word_count < strlen(word))
    {
       // helper_delete ()
    }
    */



    return false;
}

void tnode_test()
{
    tnode_insertword(&pWordList->pool[0], "mukesh");
    tnode_insertword(&pWordList->pool[0], "singh");

    tnode_insertword(&pWordList->pool[0], "mantu");
    tnode_insertword(&pWordList->pool[0], "man");
    tnode_insertword(&pWordList->pool[0], "mant");
    tnode_insertword(&pWordList->pool[0], "singh");
    tnode_insertword(&pWordList->pool[0], "mantusingh");
    tnode_insertword(&pWordList->pool[0], "a");
    cout << tnode_searchword(&pWordList->pool[0], "mantu") << endl;
    cout << tnode_searchword(&pWordList->pool[0], "mant") << endl;
    cout << tnode_searchword(&pWordList->pool[0], "singh") << endl;

    tnode_deleteword(&pWordList->pool[0], "mantu");

    cout << tnode_searchword(&pWordList->pool[0], "mantu") << endl;
    cout << tnode_searchword(&pWordList->pool[0], "man") << endl;
    cout << tnode_searchword(&pWordList->pool[0], "mant") << endl;
    cout << tnode_searchword(&pWordList->pool[0], "mantusingh") << endl;
    cout << tnode_searchword(&pWordList->pool[0], "a") << endl;
    cout << tnode_searchword(&pWordList->pool[0], "b") << endl;
    cout << tnode_searchword(&pWordList->pool[0], "m") << endl;

}


/* options this program is used with */
int selectoption(int argc, char** argv)
{
    if (strcmp(argv[1], "insert") == 0)
        return INSERT_WORD;
    else if (strcmp(argv[1], "search") == 0)
        return SEARCH_WORD;
    else if (strcmp(argv[1], "delete") == 0)
        return DELETE_WORD;
    else
        return 0;
}


/* initializes the semaphore using the SETVAL command in a semctl call. */

static int set_semvalue(void)
{
    union semun sem_union;

    sem_union.val = 1;
    if (semctl(sem_id, 0, SETVAL, sem_union) == -1)
        return(0);
    return(1);
}

/* remove semaphore using IPC_RMID command in as semctl call */

static void del_semvalue(void)
{
    union semun sem_union;

    if (semctl(sem_id, 0, IPC_RMID, sem_union) == -1)
        fprintf(stderr, "Failed to delete semaphore\n");
}

/* semaphore_p changes the semaphore by -1 (waiting). */
static int semaphore_p(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = -1; /* P() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_p failed\n");
        return(0);
    }
    return(1);
}

/* semaphore_v is similar changes the semaphore by 1 (available). */
static int semaphore_v(void)
{
    struct sembuf sem_b;

    sem_b.sem_num = 0;
    sem_b.sem_op = 1; /* V() */
    sem_b.sem_flg = SEM_UNDO;
    if (semop(sem_id, &sem_b, 1) == -1) {
        fprintf(stderr, "semaphore_v failed\n");
        return(0);
    }
    return(1);
}

/* options to this program is called with */
int shared_memory_op(int action, const char* str)
{
    struct sembuf sem_buf;
    //struct semid_ds buf;

    sem_id = semget(SEM_KEY_ID, 1 /*2*/, IPC_CREAT | IPC_EXCL | 0666);
    /* Got the semaphore */
    if (sem_id >= 0) { /* first process */
        set_semvalue();
    }
    else if (errno == EEXIST) { /* other process already created it so attach it */
        sem_id = semget(SEM_KEY_ID, 1, 0);
        if (sem_id < 0) {
            perror("Semaphore semget: ");
            return 0;
        }
    }

    switch (action)
    {
    case INSERT_WORD:  /* insert a word in shared memory */

        if (!semaphore_p())
            exit(EXIT_FAILURE);
      /*  if (tnode_searchword(&pWordList->pool[0], str))
            cout << "Duplicate word: " << str << " : not inserting it." << endl;
        else*/
            if (tnode_insertword(&pWordList->pool[0], str) == 1)
			{
				cout << "Duplicate word: " << str << " : Already exists." << endl;
			}
			else
			{
				cout << "Inserted word: " << str << endl;
			}
				

        if (!semaphore_v())
            exit(EXIT_FAILURE);

        break;
    case SEARCH_WORD:  /* search for a word */
        if (tnode_searchword(&pWordList->pool[0], str))
            cout << "Found word: " << str << endl;
        else
            cout << "Not Found word: " << str << endl;
        break;
    case DELETE_WORD:  /* delete  a word */
    {
        int temp = pWordList->npool;
        size_t curr_idx = pWordList->head;
        tnode* pt;
        size_t tempval = 0;
        int found = 0;

        if (!semaphore_p())
            exit(EXIT_FAILURE);
		if (tnode_deleteword(&pWordList->pool[0], str) == true)
			found =1;

        if (!semaphore_v())
           exit(EXIT_FAILURE);

        if (!found)
            cout << "not found word, deletion not done :  " << str << endl;
		else
			cout << "Deleted word :  " << str << endl;
    }
    break;
    default: /* not a valid option */
        break;
    }
    return 0;
}


int main(int argc, char** argv)
{
    int shmid;
    int action;
    int key_id = 0;
    char str[20];
    if (argc != 3)
    {
        cout << "\n Valid options: \n";
        cout << "<prog name> <insert|search|delete> <word> \n";
        return 0;
    }
    action = selectoption(argc, argv);

    // it is assumed that test file exist in current directory. If not create one.
    // currently ftok code is commented  so that this application is not restricted
    // to file name and can be run from anywhere.
    //key_id = ftok("test", 10);
    //shmid = shmget(key_id, sizeof(WordList_tnode), 0);  

    shmid = shmget(SHM_KEY_ID, sizeof(WordList_tnode), 0);
    if (shmid == -1)
    {
        shmid = shmget(SHM_KEY_ID, sizeof(WordList_tnode), IPC_CREAT | 0660);
        if (shmid < 0)
        {
            perror("shmget failed: ");

            exit(1);
        }
        pWordList = (WordList_tnode*)shmat(shmid, NULL, 0);
        if (pWordList == (void*)(-1))
            exit(1);

        pWordList->npool = 0;
		// initialize root node
		pWordList->pool[0].is_word = false;
		pWordList->pool[0].isddeleted = false;
		for (int i = 0; i < MAX_UNIQUE_LITERAL; i++)
		{
			pWordList->pool[0].ch[i] = NULL;
		}
		pWordList->pfree++;
        pWordList->npool++;

    }
    else
    {
        pWordList = (WordList_tnode*)shmat(shmid, NULL, 0);
        if (pWordList == (void*)(-1))
            exit(1);
    }


    if (action)
    {
        shared_memory_op(action, argv[2]);
    }
    else
    {
        cout << "\n Valid options: \n";
        cout << "<prog name> <insert|search|delete> <word> \n";
        return 0; /* not a valid operation: [insert | search | delete] */
    }

    cout << "\n happy world \n";
    //    cin >> action;

    shmdt(pWordList);

    return 0;
}



