Class Dictionary represent shared memory size.
Class do not contain any memory which will grow/shrink on run time. So eventally shared memory size is set to sizeof(Dictionary).

This constrain applied due to assumption as follows -
1. There will be maximum MAX_WORD_COUNT (1 million) words in dictionary.
2. Char max size will be MAX_WORD_SIZE(20)  
3. All words will start with alphabetic character (a-z/A-Z)only.

Implementation details -
Class Dictionary used to hold indivisual sub-dictionary(char SingleCharDictionary) per character. Used different sub-dictionary object per character helps to achieve parallelism at character level. So that user can insert words starting with different character simultaneously.

Class SimpleHash provides hashing to store dictionary words. Using simple hash function based on small prime number. Hash store node index instead of actual node pointer. To handle collition situation, each node provides next index to provide hash chaining functionality.

Alongwith per character dictionary, class pre-allocate memory to hold max word allowed in dictionary.
This memory is maintain in class FreeNodePool. Every new word inserted to dictionary will get space/node from this node pool. And on deletion node return to node pool.

Class ShedMemory used to create, map and release shared memory. This class will create new shared memory if memory is not already allocated. Else map to pre-allocated shared memory.

How to run :-
$> make clean
$> make
$> ./dict -i sagar
$> ./dict -r
