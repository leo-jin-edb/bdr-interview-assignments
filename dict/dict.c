#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<sys/types.h>
#include<errno.h>

#define HASH_MAX_INDEX 101
#define MAX_SIZE_IN_BYTES 4096 // 4KB
#define SHM_KEY 7777
#define MAX_CONC_READ 100
#define CALC_ADDR(base,offset) ((unsigned long)((unsigned long)base + (unsigned long) offset))

typedef struct HashTable{
  unsigned long total_bytes;
  unsigned long used_bytes;
  unsigned long free_bytes;
  unsigned long offset;
  unsigned long wc_fragment_chain_offset;
  unsigned long fragment_count;
  unsigned long word_count;
  unsigned long hash_bucket[HASH_MAX_INDEX];
} HashTable;

typedef struct WordChain{
  unsigned long next;
  unsigned long len;
  unsigned long word_offset;
}WordChain;

union sem_union {
  int val;
  struct semid_ds *buf;
  unsigned short  *array;
}sem_union;

struct sembuf sem_buf;
int sem_id;

int read_lock(){
  struct sembuf r = { 0, -1, SEM_UNDO};
  if(semop(sem_id,&r,1) < 0) 
    printf("READ_LOCK: error = %d",errno);
  return errno;
}

int read_unlock(){
  struct sembuf r = { 0, +1, SEM_UNDO};
  if(semop(sem_id,&r,1) < 0) 
    printf("READ_UNLOCK: error = %d",errno);
  return errno;
}

int write_lock(){
  struct sembuf w = { 0, -1 * MAX_CONC_READ, SEM_UNDO};
  if(semop(sem_id,&w,1) < 0){
    printf("WRITE_LOCK: error = %d",errno);
  }
  return errno;
}

int write_unlock(){
  struct sembuf w = { 0, +1 * MAX_CONC_READ, SEM_UNDO};
  if(semop(sem_id,&w,1) < 0) 
    printf("WRITE_UNLOCK: error = %d",errno);
  return errno;
}

void help(){
  printf("Bad command!!\n");
  printf("Command help-> ");
  printf("dict {insert | search | delete} <word>\n");
}

void stats(HashTable *ht_ptr){
  printf("========MEM-STATS========\n");
  printf("TotalBytes = %lu \n Offset = %lu \n UsedBytes = %lu \n FreeBytes = %lu \n FragmentOffset = %lu \n FragmentCount = %lu \n WordCount = %lu \n", 
      ht_ptr->total_bytes,ht_ptr->offset, ht_ptr->used_bytes, ht_ptr->free_bytes, ht_ptr->wc_fragment_chain_offset,ht_ptr->fragment_count,ht_ptr->word_count);
  printf("========END-STATS========\n");
}

int init_hash_table(HashTable **h_ptr, unsigned long size){
  int id = shmget((key_t)SHM_KEY,size,0666|IPC_CREAT);
  int e;
  union sem_union sem_val;
  if(id == -1){
    return -1;
  }

  *h_ptr = shmat(id,NULL,0);
  if(id == -1){
    return -2;
  }

  sem_id = semget((key_t)SHM_KEY,1,0666|IPC_CREAT); 
  if(sem_id < 0){
    printf("SEMGET: error = %d",errno);
    return errno;
  }

  sem_val.val = MAX_CONC_READ;
  if(semctl(sem_id, 0, SETVAL, sem_val) < 0){  
    // SETVAL is a macro to specify value of the semaphore
    printf("SEMCTL: errno = %d",errno);
    return errno;
  }
  if(write_lock() < 0){
    printf("WRITE_LOCK: error = %d",errno);
    return -1;
  }
  if((*h_ptr)->word_count == 0){
    (*h_ptr)->total_bytes = size;
    (*h_ptr)->used_bytes = sizeof(HashTable);
    (*h_ptr)->free_bytes = size - sizeof(HashTable);
    (*h_ptr)->offset = (*h_ptr)->used_bytes;
  }
  if(write_unlock() < 0){
    printf("WRITE_UNLOCK: error = %d",errno);
    return -1;
  }
  return 0;
}

unsigned long index_of_string(char *str){
  int i = 0;
  int sum = 0;

  while(str[i] != '\0'){

    sum = sum + (int) str[i++];
  }

  return (unsigned long)(sum % HASH_MAX_INDEX);
}



int insert(HashTable *ht_ptr, char *str){
  unsigned long index = index_of_string(str);
  unsigned int len = strlen(str);
  unsigned long word_offset = (unsigned long) ht_ptr->hash_bucket[index];
  WordChain *new_wc_ptr = NULL;

  if(ht_ptr->free_bytes < CALC_ADDR(sizeof(WordChain), len + 1)){
    printf("Out of memory insert is not possible\n");
    return -4;
  }
  new_wc_ptr = (WordChain*) CALC_ADDR(ht_ptr, ht_ptr->offset);
  new_wc_ptr->next = word_offset;
  new_wc_ptr->len = len;
  ht_ptr->hash_bucket[index] = (unsigned long)ht_ptr->offset;
  new_wc_ptr->word_offset = (unsigned long)CALC_ADDR(ht_ptr->offset, sizeof(WordChain));
  strcpy((char*)CALC_ADDR(ht_ptr , new_wc_ptr->word_offset),str);

  // Memory Management section 
  ht_ptr->word_count++;
  ht_ptr->offset = CALC_ADDR(ht_ptr->offset , CALC_ADDR(sizeof(WordChain) , len+1));
  ht_ptr->used_bytes = ht_ptr->used_bytes + CALC_ADDR(sizeof(WordChain) , len+1);
  ht_ptr->free_bytes = ht_ptr->free_bytes - CALC_ADDR(sizeof(WordChain) , len+1); 
  printf("New Word = %s, len = %d, indexed = %d \n", 
      CALC_ADDR(ht_ptr,new_wc_ptr->word_offset), new_wc_ptr->len, index);
}

char* search(HashTable *ht_ptr, char *str){
  unsigned long index = index_of_string(str);
  unsigned long key_len = strlen(str);
  WordChain *wc_ptr = (ht_ptr->hash_bucket[index] == 0? NULL: 
      (WordChain*)CALC_ADDR(ht_ptr , ht_ptr->hash_bucket[index]));
  WordChain *temp = wc_ptr;

  while(temp != NULL){
    if(key_len == wc_ptr->len){

      if(strcmp((char*)CALC_ADDR(ht_ptr , temp->word_offset),str) == 0) 
        return str;
      else 
        temp = temp->next == 0? NULL: (WordChain*)CALC_ADDR(ht_ptr , temp->next);
    }else
      temp = temp->next == 0? NULL: (WordChain*)CALC_ADDR(ht_ptr , temp->next);
  }

  return NULL;
}

char* delete(HashTable *ht_ptr, char *str){
  unsigned long index = index_of_string(str);
  unsigned long key_len = strlen(str);
  WordChain *wc_ptr = (ht_ptr->hash_bucket[index] == 0? NULL: 
      (WordChain*)CALC_ADDR(ht_ptr , ht_ptr->hash_bucket[index]));
  WordChain *temp = wc_ptr;
  void *prev = 0;
  void *holder = 0;
  WordChain *temp_prev = 0;

  while(temp != NULL){
    if(key_len == wc_ptr->len){
      if(strcmp((char*)CALC_ADDR(ht_ptr , temp->word_offset),str) == 0){
        holder = (void*)ht_ptr->wc_fragment_chain_offset;
        ht_ptr->wc_fragment_chain_offset = (unsigned long)temp - (unsigned long)ht_ptr;
        if(prev == 0){
          ht_ptr->hash_bucket[index] = temp->next;
        }else{
          temp_prev = (WordChain*)CALC_ADDR(ht_ptr,prev);
          temp_prev->next = temp->next;
        }
        temp->next = (unsigned long)holder;
        ht_ptr->fragment_count++;
        ht_ptr->word_count--;
        return str;
      }
      else{
        prev = temp != NULL?(void*)((unsigned long)temp - (unsigned long)ht_ptr):NULL;
        temp = temp->next == 0? NULL: (WordChain*)CALC_ADDR(ht_ptr , temp->next);
      }
    }else{
      prev = temp != NULL?(void*)((unsigned long)temp - (unsigned long)ht_ptr):NULL;
      temp = temp->next == 0? NULL: (WordChain*)CALC_ADDR(ht_ptr , temp->next);
    }
  }

  return NULL;
}


int main(int arg_count, char **arg_str){
  char *str;
  int err;
  HashTable *h_ptr = NULL;
  err = init_hash_table(&h_ptr,MAX_SIZE_IN_BYTES);
  if(err != 0){
    return err;
  }
  stats(h_ptr);

  //printf("argCount = %d, arg0=%s, arg1=%s,arg2=%s\n",arg_count,arg_str[0],arg_str[1],arg_str[2]);

  if(arg_count != 3){
    help();
    return 0;
  }else if(strcmp(arg_str[1], "insert") == 0){
    if(err = write_lock() < 0) return err;
    str = search(h_ptr,arg_str[2]);
    if(str == NULL)
      insert(h_ptr,arg_str[2]);
    stats(h_ptr);
    if(err = write_unlock() < 0) return err;
    return 0;
  }else if(strcmp(arg_str[1], "search")  == 0) {
    if(err = read_lock() < 0) return err;
    str = search(h_ptr,arg_str[2]);
    if(err = read_unlock() < 0) return err;
    printf("Search string found = %s\n",str);
    return 0;
  }else if(strcmp(arg_str[1], "delete") == 0){
    if(err = write_lock() < 0) return err;
    str = delete(h_ptr,arg_str[2]);
    if(err = write_unlock() < 0) return err;
    printf("Deleted string = %s\n",str);
    return 0;
  }else{
    help();
    return 0;
  }

  shmdt(h_ptr);
}

