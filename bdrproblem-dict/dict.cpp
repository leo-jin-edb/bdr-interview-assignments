#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <iostream>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <thread>
#include <chrono>
#include <string.h>

const std::string SEG_FILE =  "_segf_1g";
const int SEG_FILE_SIZE = 1024 * 1024 * 1024; //1Gb
const int MAP_SEM_KEY = 123456;
const int DATA_SEM_KEY = 123457;
const int HASH_MAP_OFFSET = 0;
const int DATA_OFFSET =  1024 * 1024; 
const int HASH_BUCKETS = 65521;

union semun {
    int val;
    struct semid_ds *buf;
    unsigned short int *array;
    struct seminfo *__buf;
};

int binary_semaphore_allocation (key_t key, int sem_flags)
{
    return semget (key, 1, sem_flags);
}

int binary_semaphore_initialize (int semid)
{
    union semun argument;
    unsigned short values[1];
    values[0] = 1;
    argument.array = values;
    return semctl (semid, 0, SETALL, argument);
}

int binary_semaphore_wait (int semid)
{
    struct sembuf operations[1];
    /* Use the first (and only) semaphore. */
    operations[0].sem_num = 0;
    /* Decrement by 1. */
    operations[0].sem_op = -1;
    /* Permit undo’ing. */
    operations[0].sem_flg = SEM_UNDO;

    return semop (semid, operations, 1);
}

int binary_semaphore_post (int semid)
{
    struct sembuf operations[1];
    /* Use the first (and only) semaphore.*/
    operations[0].sem_num = 0;
    /* Increment by 1. */
    operations[0].sem_op = 1;
    /* Permit undo’ing. */
    operations[0].sem_flg = SEM_UNDO;
    return semop(semid, operations, 1);
}

class Lock {
    int sem_id;
    key_t key;
public:
    Lock(key_t key);
    void acquire();
    void release();
};

Lock::Lock(key_t key) {
    this->key = key;
    this->sem_id = binary_semaphore_allocation(key, IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);

    if (this->sem_id == -1) {
        if (errno != EEXIST) {
            exit(errno);
        }
        this->sem_id = binary_semaphore_allocation(key, S_IRUSR | S_IWUSR);

        if (this->sem_id == -1) {
            exit(errno);
        }
    } else {
       binary_semaphore_initialize(sem_id);
    }
    //std::cout << "semaphore_id = " << sem_id << std::endl;
}

void Lock::acquire() {
    binary_semaphore_wait(this->sem_id);
}

void Lock::release() {
    binary_semaphore_post(this->sem_id);
}

class SharedMemoryManager {
    private:
        std::string seg_file;
        void *file_memory;
        int length;
        int fd;
    public:
    SharedMemoryManager(std::string seg_file, int length);
    ~SharedMemoryManager();
    void *get_root_mem_ptr() {
        return file_memory;
    };
};

SharedMemoryManager::SharedMemoryManager(std::string seg_file, int length) {
    //std::cout << "file_name = " << seg_file << " length = " << length << std::endl; 
    fd = open(seg_file.c_str(), O_RDWR, S_IRUSR | S_IWUSR);
    if (fd == -1) {
        std::cout << "failed map file " << errno << std::endl;
        return;
    }     
    this->file_memory = mmap(0, length, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
}

SharedMemoryManager::~SharedMemoryManager() {
    munmap(file_memory, length);
}

struct Data {
    int next_offset;
    int size;
    char word[1];
};

struct DictDataInfo {
    int cur_data_offset;
};

class Dict {
private:
    void *mem_ptr;
    int *map_ptr;
    void *data_seg_ptr;
    Lock *hash_map_lock;
    Lock *data_lock;
    SharedMemoryManager *shmm;
public:
    Dict();
    void insert(std::string word);
    bool del(std::string word);
    bool search(std::string word);
};

Dict::Dict() {
    shmm = new SharedMemoryManager(SEG_FILE, SEG_FILE_SIZE);
    data_lock = new Lock(DATA_SEM_KEY);
    hash_map_lock = new Lock(MAP_SEM_KEY);
    this->mem_ptr = shmm->get_root_mem_ptr();
    this->map_ptr = (int32_t *)((char *)(mem_ptr) + HASH_MAP_OFFSET);
    data_seg_ptr = (char *)(mem_ptr) + DATA_OFFSET;
}

void Dict::insert(std::string word) {
    std::size_t word_hash = std::hash<std::string>{}(word);

    //int map_ind = word_hash & 0x0000FFFF; // get lower 16 bits
    int map_ind = word_hash % HASH_BUCKETS; // get lower 16 bits

    data_lock->acquire();
    DictDataInfo *dict_data_info = (DictDataInfo *)(data_seg_ptr);

    if (dict_data_info->cur_data_offset == 0) {
        dict_data_info->cur_data_offset = sizeof(DictDataInfo);
    }

    int offset = dict_data_info->cur_data_offset;
    Data *data = (Data *)((char *)data_seg_ptr + offset);
    data->next_offset = 0;
    data->size = word.length();
    memcpy(&(data->word), word.c_str(), word.length());
    int next_offset = offset + (2 * sizeof(int) + data->size);
    dict_data_info->cur_data_offset = next_offset;
    //std::this_thread::sleep_for(std::chrono::seconds(word.length()));
    data_lock->release();

    hash_map_lock->acquire();
    int cur_offset = map_ptr[map_ind];
    if (cur_offset != 0) {
        data->next_offset = cur_offset;
    }
    map_ptr[map_ind] = offset;
    //std::this_thread::sleep_for(std::chrono::seconds(word.length()));
    hash_map_lock->release();
}

bool Dict::search(std::string word) {
    std::size_t word_hash = std::hash<std::string>{}(word);

    //int map_ind = word_hash & 0x0000FFFF; // get lower 16 bits
    int map_ind = word_hash % HASH_BUCKETS; // get lower 16 bits
    bool found = false;
    hash_map_lock->acquire();
    int next_offset = map_ptr[map_ind];
    while (next_offset != 0) {
        Data *data = (Data *)((char *)data_seg_ptr + next_offset);
        if (data->size == word.length()) {
            if (memcmp(&data->word, word.c_str(), data->size) == 0) {
                found = true;
                break;
            }
        }
        next_offset = data->next_offset;
    }
    //std::this_thread::sleep_for(std::chrono::seconds(word.length()));
    hash_map_lock->release();
    return found;
}

bool Dict::del(std::string word) {
    std::size_t word_hash = std::hash<std::string>{}(word);

    //int map_ind = word_hash & 0x0000FFFF; // get lower 16 bits
    int map_ind = word_hash % HASH_BUCKETS; // get lower 16 bits
    bool found = false;
    hash_map_lock->acquire();
    int next_offset = map_ptr[map_ind];
    int *prev_offset = &(map_ptr[map_ind]);
    while (next_offset != 0) {
        Data *data = (Data *)((char *)data_seg_ptr + next_offset);
        if (data->size == word.length()) {
            if (memcmp(&data->word, word.c_str(), data->size) == 0) {
                *prev_offset = data->next_offset;
                data->next_offset = -1;
                found = true;
                break;
            }
        }
        prev_offset = &(data->next_offset);
        next_offset = data->next_offset;
    }
    //std::this_thread::sleep_for(std::chrono::seconds(word.length()));
    hash_map_lock->release();
    return found;
}

/*
    This program implements dict that can be shared by multiple processes concurrently. 
    INSERT/SEARCH/DELETE operations are supported. 

    Following are the implementation details, current limits and
    scope for improvements.

    Implementation Details
    ----------------------
    Shared memory: Memory sharing between multiple processes is implemented 
    through shared memory mapped file. 
    A precreated file (_segf_1g) filled with zeros is created outside the program  as setup.

    Locking:
    Locking between multiprocess concurrent operations is done through semaphores. Currently 2 semapohres
    are used - one for locking operations on data portion and other for operations on hash table.
        
    Dict Memory Layout:
    The dict is built in 1 GB of memory mapped file. The memory is divided into 3 portions -  
    hash table (1MB), DataInfo (few bytes), data (remaining memory).

    ---------------------------------------------------
    | Hash table | data info | data .......           |
    ---------------------------------------------------

    Hash Table:
    ~64K buckets. A word is hashed to get 64 bit hash using the hash function provided by 
    c++ std library. The 64 bit hash function is mapped to 64K buckets using prime number modulo. 
    The bucket contains the offset in the data portion where data is copied. If two words belong
    to same bucket, then the words are chained linearly.
    
    Data Info:
    Holds the offset in data portion where data can be appended to in data portion of memory.
    
    Data:
    Each word is copied in data portion with following format
    -----------------------------
    |next_offset|  size    |word|
    -----------------------------
    next_offset - when > 0, indicates the offset of a word with same hash. When 0 indicates that there is no
    word with same hash. When -1 indicates that the word is deleted.


    Limitations of current implementation:
    - As defined in the problem this dict has to hold 1 million words. With the choosen 1G memory
      to share between processes, the average word size can be 1K.
    - Memory limit check is not done.
    - Delete operation is a soft delete where a word is not physically delete in memory. This leads
      to accumulation of unused memory.
    - Locking on entire hash table. This affects the read and write concurrent operations latency.
    - If there are words that has many hash synonyms, then the search/delete operation performance 
    can be bad due to linear search within the chain of hash synonyms. However with the distinct words
     to be 1million , the probability of duplicate with 64 bit hash function can be very very less.
    
    Thoughts on improving or solving above limitations:
    - Implement data portion with multiple shared memory mapped files to increase the memory for
      data. 
    - Add memory limit check.
    - To avoid unused memory problem add a daemon that does compaction. Issues related to concurrent
      write/delete/search queries while compaction in progress have to be taken care of
    - 2 levels of hash table. First level hash table has fewer buckets (like 8) where each bucket has its
      own lock. This reduces the coarser lock to finer lock improving the performance of concurrent
      operations.
   

 */

int main(int argc, char *argv[]) {
    Dict dict;
    std::string op;
    std::string word;

    if (argc < 3) {
        std::cout << "invalid arguments.";
        std::cout << "dict insert|search|delete <word>" << std::endl;
        return -1;
    }

    op = argv[1];
    word = argv[2];

    //std::cout << "op = " << op << " word = " << word << std::endl;

    if (op == "insert") {
        if (!dict.search(word)) {
            dict.insert(word);
            std::cout << "INSERTED " << word << std::endl;
        } else {
            std::cout << "DUPLICATE " << word << std::endl;
        }
    } else if (op == "delete") {
        if (dict.del(word)) {
            std::cout << "DELETED " << word << std::endl;
        } else {
            std::cout << "NOT DELETED " << word << std::endl;
        }
    } else if (op == "search") {
        if (dict.search(word)) {
            std::cout << "FOUND " << word << std::endl;
        } else {
            std::cout << "NOT_FOUND " << word << std::endl;
        } 
    }
    return 1;
}
