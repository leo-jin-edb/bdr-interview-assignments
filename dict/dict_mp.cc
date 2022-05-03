/* dict - Concurrent dictionary (multiprocessing) */
/* Author : Mukesh Kr. Singh                      */
/* Date:    1st March 2022                        */
/* Comments: this program uses shared memory      */
/*   semaphore to implement concurrent dictionary */
/* Compilation: This code is compiled and tested  */
/*  on Linux using gcc version 9.3.0              */
/*  g++ -o dict dict_mp.cc                        */
/* ************************************************/
/**************************************************
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


typedef struct WordNode WordNode;
typedef struct WordList WordList;

#define MAX_WORDLEN 32                   // Max. word length (assumed 32 bytes)
#define MAX_WORDNODE 1000000             // Max. number of word nodes

#define SHM_KEY_ID 0x23456
#define SEM_KEY_ID 0x24567

#define WORDNULL  -1 // (MAX_WORDLEN + 1)       // NULL value

// structure to store a word and its metadata
struct WordNode {
    char word[MAX_WORDLEN];
    //unsigned short length_occupied;
    bool isdeleted;
    size_t next;
};

// memory pool to store words 
struct WordList {
    WordNode pool[MAX_WORDNODE];      // fixed-size space for nodes
    size_t npool;                     // used space in pool
    size_t pfree;                     // pointer to re-use freed nodes
    size_t head;                      // global list head
};

union semun {
   int              val;    /* Value for SETVAL */
   struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
   unsigned short  *array;  /* Array for GETALL, SETALL */
   struct seminfo  *__buf;  /* Buffer for IPC_INFO
                               (Linux-specific) */
};

WordList *pWordList;

static int sem_id;

typedef enum op{
	INSERT_WORD = 1,
	SEARCH_WORD,
	DELETE_WORD
} op;


/* allocates memory to store a word from memory pool */
WordNode *wordnode_alloc(void)
{
    if (pWordList->pfree != WORDNULL) {
        WordNode *node = pWordList->pool + pWordList->pfree;

        pWordList->pfree = pWordList->pool[pWordList->pfree].next;
        return node;
    } else {
        if (pWordList->npool < MAX_WORDNODE) return &pWordList->pool[pWordList->npool++];
    }

    return NULL;
}

/* free the allocated memory to node */
void wordnode_free(WordNode *node)
{
    if (node) {
        node->next = pWordList->pfree;
        pWordList->pfree = node - pWordList->pool;
    }
}
/* returns corresponding word node from index */ 
WordNode *wordnode(size_t index)
{
    return (index == WORDNULL) ? NULL : pWordList->pool + index;
}

/* return next node */
WordNode *wordnode_next(const WordNode *node)
{
    return wordnode(node->next);
}

/* insert node  into to the memory pool */
WordNode *wordnode_push(size_t *head, const char *str)
{
    WordNode *node = wordnode_alloc();

    if (node) {
        strncpy(node->word, str, sizeof(node->word));
        node->next = *head;
        *head = node - pWordList->pool;
    }

    return node;
}

/* pop up node from the memory pool which can be used to display it */
size_t wordnode_pop(size_t *head)
{
    if (*head != WORDNULL) {
        size_t next = pWordList->pool[*head].next;
      return next;
    }
    return 0;
}

/* options this program is used with */
int selectoption(int argc, char **argv)
{
    if(strcmp (argv[1], "insert") == 0)
        return INSERT_WORD;
    else if (strcmp (argv[1], "search") == 0)
        return SEARCH_WORD;
    else if (strcmp (argv[1], "delete") == 0)
        return DELETE_WORD;
    else
        return 0;
}

/* serach a word in the dictionary */
int wordnode_search(const char *str)
{
    int temp = pWordList->npool;
    size_t curr_idx= pWordList->head;
    size_t tempval=0;     
    int found=0;
    WordNode *pt;
    
    while (temp-- > 0)
    {
        pt = wordnode(curr_idx);
        if (pt && (strcmp (pt->word, str) == 0 ) && pt->isdeleted == 0)
        {
            found = 1;
            break;
        }
        tempval=wordnode_pop(&curr_idx);
        curr_idx=tempval;
    }
    return found;    
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
int shared_memory_op(int action, const char *str)
{
    struct sembuf sem_buf;
    //struct semid_ds buf;

    sem_id = semget(SEM_KEY_ID, 1 /*2*/, IPC_CREAT | IPC_EXCL | 0666);
    /* Got the semaphore */
    if (sem_id >= 0) { /* first process */
      set_semvalue();
    } else if (errno == EEXIST) { /* other process already created it so attach it */
      sem_id = semget(SEM_KEY_ID, 1, 0);
      if (sem_id < 0) {
         perror("Semaphore semget: ");
         return 0;
      }
    }
   
    switch(action)
    {
        case INSERT_WORD:  /* insert a word in shared memory */
                if (!semaphore_p())
                    exit(EXIT_FAILURE);
                if (wordnode_search(str))
                    cout << "Duplicate word: " << str << " : not inserting it." << endl;
                else
                    wordnode_push(&pWordList->head, str);                
                if (!semaphore_v())
                    exit(EXIT_FAILURE);
                break;
        case SEARCH_WORD:  /* search for a word */
                if (wordnode_search(str))
                    cout << "Found word: " << str << endl;
                else
                    cout << "Not Found word: " << str << endl;
                break;
        case DELETE_WORD:  /* delete  a word */
                {
                int temp = pWordList->npool;
                size_t curr_idx= pWordList->head;
                WordNode *pt;
                size_t tempval=0;     
                int found=0;          
                if (!semaphore_p())
                    exit(EXIT_FAILURE);                
                while (temp-- > 0) {
                    pt = wordnode(curr_idx);
                    if ((strcmp (pt ->word, str) == 0 ) && pt->isdeleted == 0)
                    {
                        cout << "deleted word :  " << str << endl;
                        found = 1;
                        pt->isdeleted = 1;
                        break;
                    }
                    tempval=wordnode_pop(&curr_idx);
                    curr_idx=tempval;
                }
                if (!semaphore_v())
                    exit(EXIT_FAILURE);
                if(!found)
                     cout << "not found word, deletion not done :  " << str << endl;
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
    int key_id=0;
    char str[20];
    if (argc != 3)
    {
        cout <<"\n Valid options: \n";
        cout << "<prog name> <insert|search|delete> <word> \n";
        return 0;
    }
    action=selectoption(argc, argv);
    
    // it is assumed that test file exist in current directory. If not create one.
    // currently ftok code is commented  so that this application is not restricted
    // to file name and can be run from anywhere.
    //key_id = ftok("test", 10);
    //shmid = shmget(key_id, sizeof(WordList), 0);  

    shmid = shmget(SHM_KEY_ID, sizeof(WordList), 0);  
    if (shmid == -1)
    {
        shmid = shmget(SHM_KEY_ID, sizeof(WordList), IPC_CREAT | 0660);    
        if (shmid < 0) 
        {
            perror ("shmget failed: ");
            
            exit(1);
        }
        pWordList = (WordList *)shmat(shmid, NULL, 0);
        if (pWordList == (void *) (-1))
            exit(1);

        pWordList->head = WORDNULL;
        pWordList->pfree = WORDNULL;
        pWordList->npool = 0;
        
    }
    else
    {
        pWordList = (WordList *)shmat(shmid, NULL, 0);
        if (pWordList == (void *) (-1))
            exit(1);
    }

    
    if (action)
    {
        shared_memory_op(action, argv[2]);
    }
    else
    {
        cout <<"\n Valid options: \n";
        cout << "<prog name> <insert|search|delete> <word> \n";
        return 0; /* not a valid operation: [insert | search | delete] */
    }    
    
    int temp = pWordList->npool;
    size_t curr_idx= pWordList->head;
    size_t tempval=0;
    WordNode *pt = NULL;

    while ( temp-- > 0 ) {
        pt = wordnode(curr_idx);
        if (pt && !pt->isdeleted)
            puts(wordnode(curr_idx)->word);
        tempval=wordnode_pop(&curr_idx);
        curr_idx=tempval;
    }
    
    cout <<"\n happy world \n";
//    cin >> action;
    
    shmdt(pWordList);

    return 0;
}

