#ifndef _SIMPLE_HASH_H_
#define _SIMPLE_HASH_H_

#include "nodepool.h"
#include <string>

/*
Class provides hashing to store dictionary words. Using simple hash function based on small
prime number. Hash store node index instead of actual node pointer. To handle collition situation,
each node provides next index to provide hash chaining functionality.
*/
class SimpleHash
{
public:
    SimpleHash();
    ~SimpleHash() {}
    void Init(FreeNodePool* pool);
    
    bool insertword(std::string const& str);
    bool searchword(string const& str);
    bool deleteword(string const& str);
    void Print();
	
private:
    bool isPresent(const char* str, unsigned int hashval);
    unsigned int computeHash(const char* str);    //Using polynomial rolling hash
	
    unsigned int  _simpleHashTable[MAX_WORD_COUNT/2];
    unsigned int  _hashSize;
    FreeNodePool* _nodePool;
};

#endif
