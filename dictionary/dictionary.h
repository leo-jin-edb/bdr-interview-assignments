#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <string>
#include "nodepool.h"
#include "simplehash.h"

using namespace std;
#define MAX_CHAR 26

/*
class used to represent dictionary for single character.
Used different dictionary object per character to achieve parallelism at character level.

Here dictionary can be used as reader/writer buffer. Multiple search (in-parallel) can be possible
on dictionary but only single insert and delete operation is allowed at-a-time.
*/
class SingleCharDictionary 
{
public:
    SingleCharDictionary();
    ~SingleCharDictionary() { }
    void Init(FreeNodePool* _nodePool);
    void SetNodePool(FreeNodePool* _nodePool);
    
    bool insertword(string const& str);
    bool searchword(string const& str);
    bool deleteword(string const& str);
    void Print();
	
private:    
    sem_t _rwMutex;        //Mutex to protect insert/delete operation
    sem_t _rMutex;         //Mutex to protect search from insert/delete 
    int   _readerCount;    //Total active reader count

    SimpleHash _charHash;
};

/*
Class represent shared memory size. Class do not contain any memory which willgro/shrink on run time.
This constrain applied due to assumption as follows -
1. There will be maximum MAX_WORD_COUNT (1 million) words in dictionary.
2. Char max size will be MAX_WORD_SIZE(20)  
3. All words will start with alphabetic character (a-z/A-Z)only.

Class used to hold indivisual sub-dictionary per character.
Alongwith per character dictionary, class pre-allocate memory to hold max word allowed in dictionary.
This memory is maintain in nodepool.
Every new word inserted to dictionary will get space/node from this node pool. And on deletion
node return to 
*/
class Dictionary
{
public:
    Dictionary() { } 
    ~Dictionary() { }
    void Init(bool alreadyExist);
    
    bool insertword(string str);
    bool searchword(string str);
    bool deleteword(string str);
    void Print();

private:
		
    SingleCharDictionary _charDictionary[MAX_CHAR];
    FreeNodePool         _nodePool;
};

#endif
