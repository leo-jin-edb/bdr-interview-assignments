#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <semaphore.h>

#define MAX_WORD_LENGTH 32
#define MAX_ENTRIES 1000000

#define INIT 0
#define CONTAINS 1
#define INSERT 2
#define DELETE 3
#define END 4

#define SET_SEM "set_semaphore"
#define SHM_ID "/shared_mem_id"

// Set API definitions

// Definition for each node in the Set object
typedef struct SetNode
{
  char key[32]; // this is our key
  // Imagine the rest of the implementation
} SetNode;

// Definition for the Set object
typedef struct Set
{
  SetNode *buckets;
} Set;

/* This will insert a string in the set. Will be a no-op
   if element already exists */
void insert_set(Set *set, char *word);

/* This will search if given string is in the set.
   Will return 1 if string is present and 0 if it is not */
int contains_set(Set *set, char *word);

/* This will remove given string from the set. Will be a
   no-op if element is not present */
void delete_set(Set *set, char *word);

int parseAction(char *action)
{
  if (strcasecmp(action, "init") == 0)
  {
    return INIT;
  }
  else if (strcasecmp(action, "insert") == 0)
  {
    return INSERT;
  }
  else if (strcasecmp(action, "search") == 0)
  {
    return CONTAINS;
  }
  else if (strcasecmp(action, "delete") == 0)
  {
    return DELETE;
  }
  else if (strcasecmp(action, "end") == 0)
  {
    return END;
  }
  return -1;
}

// This is a helper function specifically for getting
// the number of words and our set from our
// shared memory. This just reduces our boilerplate
void *getSharedMemory(size_t len)
{
  int fd;
  void *address = NULL;

  if ((fd = shm_open(SHM_ID, O_RDWR, 0666)) == -1)
  {
    perror("shm_open failed");
    return NULL;
  }

  if ((address = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0)) == MAP_FAILED)
  {
    perror("mmap failed");
    return NULL;
  }

  return address;
};

int main(int argc, char *argv[])
{
  int action;
  // Shared memory is going to have the format of num_words and then
  // the set with a max 1M number of entries. Might need to be
  // adjusted if Set has anything else inside of it.
  size_t len = sizeof(int) + MAX_ENTRIES * sizeof(SetNode);
  sem_t *sem_set;

  int fd;
  void *address;
  int *num_words;
  Set *set;
  char *word;

  if (argc < 2)
  {
    perror("Invalid number of arguments");
    return 1;
  }

  action = parseAction(argv[1]);

  if (action != INIT || action != END)
  {
    if (argc < 3)
    {
      perror("Invalid number of arguments");
      return 1;
    }

    word = argv[2];

    if (strlen(word) > 31)
    {
      perror("Given word is too big");
      return 1;
    }
  }

  switch (action)
  {
  case INIT:
    // Initialize the shared memory and our semaphores

    // Create shared memory that future processes will use
    fd = shm_open(SHM_ID, O_RDWR | O_CREAT, 0666);
    if (fd == -1)
    {
      perror("shm_open failed");
      return 1;
    }

    if (ftruncate(fd, len) == -1)
    {
      perror("ftruncate failed");
      return 1;
    }

    if ((sem_set = sem_open(SET_SEM, O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED)
    {
      perror("sem_open failed");
      return 1;
    }

    break;
  case INSERT:
    word = argv[2];
    if ((sem_set = sem_open(SET_SEM, 0)) == SEM_FAILED)
    {
      perror("sem_open failed");
      return 1;
    }

    if ((address = getSharedMemory(len)) == NULL)
    {
      return 1;
    }

    sem_wait(sem_set); /* P operation */

    /* critical section */
    num_words = (int *)address;
    set = (Set *)((char *)address + sizeof(int));

    if (*num_words < MAX_ENTRIES && !contains_set(set, word))
    {
      insert_set(set, word);
      // increment the number of words
      *num_words = (*num_words) + 1;
    }

    sem_post(sem_set); /* V operation */

    break;
  case CONTAINS:
    word = argv[2];

    if ((sem_set = sem_open(SET_SEM, 0)) == SEM_FAILED)
    {
      perror("sem_open failed");
      return 1;
    }

    if ((address = getSharedMemory(len)) == NULL)
    {
      return 1;
    }

    sem_wait(sem_set); /* P operation */

    /* critical section */
    num_words = (int *)address;
    set = (Set *)((char *)address + sizeof(int));

    sem_post(sem_set); /* V operation */

    break;
  case DELETE:
    word = argv[2];

    if ((sem_set = sem_open(SET_SEM, 0)) == SEM_FAILED)
    {
      perror("sem_open failed");
      return 1;
    }

    if ((address = getSharedMemory(len)) == NULL)
    {
      return 1;
    }

    sem_wait(sem_set); /* P operation */

    /* critical section */
    num_words = (int *)address;
    set = (Set *)((char *)address + sizeof(int));

    if (contains_set(set, word))
    {
      delete_set(set, word);
      // decrement the number of words
      *num_words = (*num_words) - 1;
    }

    sem_post(sem_set); /* V operation */

    break;
  case END:
    // clean up our semaphores and shared memory
    shm_unlink(SHM_ID);
    sem_unlink(SET_SEM);

    break;
  default:
    perror("Invalid action\n");
    return -1;
  }

  return 0;
}