#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <semaphore.h>
#ifdef LINUX
#include <stdint.h>
#endif

#define BUF_SIZE 4096


#define SEARCH 1
#define INSERT 2
#define DELETE 3

#define PRIME 1572869
#define MAX_WORD_SIZE 20

struct hashTable_t 
{
   int  cnt;
   char buf[MAX_WORD_SIZE*PRIME];
};


#define FNV_OFFSET 14695981039346656037UL
#define FNV_PRIME 1099511628211UL

// Return 64-bit FNV-1a hash for key (NUL-terminated)
// https://en.wikipedia.org/wiki/Fowler–Noll–Vo_hash_function
static uint64_t hash_key(const char* key) {
    uint64_t hash = FNV_OFFSET;
    for (const char* p = key; *p; p++) {
        hash ^= (unsigned int)(unsigned char)(*p);
        hash *= FNV_PRIME;
    }
    return hash;
}



int insert(struct hashTable_t *ht, const char *word, sem_t *sem_fd);
int delete(struct hashTable_t *ht, const char *word, sem_t *sem_fd);
int search(struct hashTable_t *ht, const char *word, sem_t *sem_fd);
void printUsage();

int main(int argc, char *argv[]) 
{
    int    rtc = 0;
    char   operation[10];
    int    opid;
    char   word[MAX_WORD_SIZE];
    const  char *shmName = "HTShmem";
    const  char *semName = "HTSem";
    int    created=0;
    int    shmfd;
    sem_t  *semfd = NULL;
    struct hashTable_t *ht;
    u_int64_t hashValue;
    u_int64_t index;
    
    switch(argc)
    {
        case 2:
            if (strncmp(argv[1], "del_shm", strlen("del_shm")))
            {
                printUsage();
                rtc = -1;
                goto done;
            }
            if (shm_unlink(shmName) < 0)
            {
                perror("Error shm_unlink(): ");
                rtc = errno;
            }
            if (close(shmfd) < 0)
            {
                perror("Error close(): ");
                rtc = errno;
            }
            if (sem_unlink(semName) < 0) {
                perror("Error sem_unlink(): ");
            }
            goto done;
        case 3:
            if (!strncmp(argv[1], "insert", 10))
                opid = INSERT;
            else if (!strncmp(argv[1], "search", 10))
                opid = SEARCH;
            else if (!strncmp(argv[1], "delete", 10))
                opid = DELETE;
            else
            {
                printUsage();
                rtc = -1;
                goto done;
            }
            strncpy(word, argv[2], MAX_WORD_SIZE);
            break;
        default:
            printUsage();
            rtc = -1;
            goto done;
    }

     /* check if shared memory segment exists */
    shmfd = shm_open(shmName, O_RDWR|O_CREAT|O_EXCL,
                      S_IRUSR|S_IWUSR|S_IXUSR);
    /* A new object is created, set the size */
    if (shmfd > 1)
    {
        if (ftruncate(shmfd, sizeof(struct hashTable_t)) == -1)
        {
            perror("Error ftruncate(): ");
            rtc = errno;
            goto done;
        }
        created = 1;
    } 
    else if (shmfd == -1)
    {
        if(errno == EEXIST)
        {
            printf("INFO: Shared memory exists\n");
            shmfd = shm_open(shmName, O_RDWR,S_IRUSR|S_IWUSR|S_IXUSR);
            if (shmfd == -1)
            {
                perror("Error shm_open() when opening existing file: ");
                rtc = errno;
                goto done;
            }
        }
        else
        {
            perror("Error creating shared mem object: ");
            rtc = errno;
            goto done;
        }
    }   
    
    // Attach to the segment to get a pointer to it.
    ht = mmap(0, sizeof(struct hashTable_t), PROT_WRITE|PROT_READ, MAP_SHARED, shmfd, 0);
    if (ht == (void *) -1)
    {
        printf("Error mmap(): ");
        rtc = errno;
        goto done;
    }
    if (created) 
    {
        memset(ht,0,sizeof(struct hashTable_t));
    }
    semfd = sem_open(semName, O_CREAT, S_IRUSR|S_IWUSR|S_IXUSR, 1);
    if (semfd == SEM_FAILED)
    {
        perror("Error sem_open(): ");
        rtc = errno;
        goto done;  
    }

    if (opid == INSERT)
    {
        rtc = insert(ht, word, semfd);
        if (rtc)
            goto done;
    }
    else if (opid == DELETE)
    {
        rtc = delete(ht, word, semfd);
        if (rtc)
            goto done;
    }
    else if (opid == SEARCH)
    {
        rtc = search(ht, word, semfd);
        if (rtc == 0)
            printf("word %s was not found\n", word);
        else if (rtc == 1)
            printf("word %s was found\n",word);
    }
    
done:
    if (semfd && semfd != SEM_FAILED)
        sem_close(semfd);
    return rtc;
}

static u_int64_t get_index(struct hashTable_t *ht, const char *word) 
{
    if (ht == NULL)
        return -1;
    uint64_t hashValue = hash_key(word);
    uint64_t index = (hashValue % PRIME)*MAX_WORD_SIZE;
    while (ht->buf[index] != 0) 
    {
        if (strncmp(&ht->buf[index],word,MAX_WORD_SIZE) == 0) 
        {
            return index;
        }
        index += MAX_WORD_SIZE;
        if (index > PRIME) 
        {
            //Wrap back to 0
            index = 0;
        }
    }
    return index;
}
/* Insert the word using a hash function, a hash function will
   will give a number for a word. This number will be converted
   to index location in the hash table by using modulo of the 
   number. Once location is pointed, insert the word. */
int insert(struct hashTable_t *ht, const char *word, sem_t *semfd)
{
    if (sem_wait(semfd) == -1)
    {
        perror("Error sem_wait() in insert()");
        return errno;
    }
    uint64_t index = get_index(ht,word);

    if (ht->buf[index] == 0)
    {
        ht->cnt++; /* for testing only */
    }
    strncpy(&ht->buf[index],word,MAX_WORD_SIZE);

    if (sem_post(semfd) == -1)
    {
        perror("Error sem_post() in insert(): ");
        return errno;
    }
   
    return index/MAX_WORD_SIZE;
}

/* delete the word using a hash function, a hash function will
   will give a number for a word. This number will be converted
   to index location in the hash table by using modulo of the 
   number, delete the word from the location */
int delete(struct hashTable_t *ht, const char *word, sem_t *semfd)
{
    if (sem_wait(semfd) == -1)
    {
        perror("Error sem_wait() in delete(): ");
        return errno;
    }
    
    uint64_t index = get_index(ht,word);
    int deleted = -1;
    if (ht->buf[index] != 0)
    {
        memset(&ht->buf[index], 0, MAX_WORD_SIZE);
        ht->cnt--; 
        deleted = index/MAX_WORD_SIZE;
    }

    if (sem_post(semfd) == -1)
    {
        perror("Error sem_post() in delete(): ");
        return errno;
    }
    return deleted;
}

/* Search the word using a hash function, a hash function will
   will give a number for a word. This number will be converted
   to index location in the hash table by using modulo of the 
   number. Get the word from this location, if it matches
   with given word return 1 else 0 */
int search(struct hashTable_t *ht, const char *word, sem_t *semfd)
{
    if (sem_wait(semfd) == -1)
    {
        perror("Error sem_wait() in search(): ");
        return errno;
    }
    
   
   uint64_t index = get_index(ht,word);
   int found = (ht->buf[index] != 0);

    if (sem_post(semfd) == -1)
    {
        perror("Error sem_post() in search(): ");
        return errno;
    }
    
    return found;
}

void printUsage()
{
    printf("\n Input error.");
    printf("\n Usage: dict search|create|delete <word>\n");
    printf("\n Usage: dict del_shm\n");
}