# Dictionary Implementation
This program implements dict that can be shared by multiple processes concurrently. INSERT/SEARCH/DELETE operations are supported. 


Following are the implementation details, current limits and thoughts on improvements.


## Implementation Details
### Shared memory 
Memory sharing between multiple processes is implemented through shared memory mapped file. A precreated file (_segf_1g) filled with zeros is created outside the program  as setup.

### Locking
Locking between multiprocess concurrent operations is done through semaphores. Currently 2 semapohres are used - one for locking operations on data portion and other for operations on hash table.  

### Dict Memory Layout
The dict is built in 1 GB of memory mapped file. The memory is divided into 3 portions -  hash table (1MB), DataInfo (4 bytes), data (remaining memory).

    ---------------------------------------------------
    | Hash table | data info | data .......           |
    ---------------------------------------------------
####  Hash Table
 uses ~64K buckets. A word is hashed to get 64 bit hash using the hash function provided by c++ std library. The 64 bit hash function is mapped to 64K buckets using prime number modulo.  The bucket contains the offset in the data portion where data is copied. If two words belong to same bucket (hash synonyms), then the words are chained linearly.
    
####  Data Info:
Holds the current offset in data portion where data can be appended to in data portion of memory.
    
#### Data:
Each word is copied in data portion with following format


    -----------------------------
    |next_offset|  size    |word|
    -----------------------------
next_offset - when > 0, indicates the offset of a word with same hash. When 0 indicates that there is no word with same hash. When -1 indicates that the word is deleted.
size - size of the word
word - actual word bytes


#### Limitations of current implementation:
* As defined in the problem this dict has to hold 1 million words. With the choosen 1G memory to share between processes, the average word size can be 1K.
* Memory limit check is not done.
* Delete operation is a soft delete where a word is not physically delete in memory. This leads to accumulation of unused memory.
* Locking on entire hash table. This affects the read and write concurrent operations latency.
* If there are words that has many hash synonyms, then the search/delete operation performance can be bad due to linear search within the chain of hash synonyms.However with the distinct words to be 1million , the probability of duplicate with 64 bit hash function can be very very less. But the modulo with prime number can increase the possibility of
synonyms.
    
#### Thoughts on improving or solving above limitations:
* Implement data portion with multiple shared memory mapped files to increase the memory for data. 
* Add memory limit check.
* To avoid unused memory problem add a daemon that does compaction. Issues related to concurrent write/delete/search queries while compaction in progress have to be taken care of.
* 2 levels of hash table. First level hash table has fewer buckets (like 8) where each bucket has its own lock. This reduces the coarser lock to finer lock improving the performance of concurrent operations.
* If the dup chain increases more than few (3 to 4) words, binary search tree structure can be implemented within the hash bucket.