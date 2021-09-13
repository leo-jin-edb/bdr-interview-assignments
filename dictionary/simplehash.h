#ifndef _SIMPLE_HASH_H_
#define _SIMPLE_HASH_H_

#include "nodepool.h"
#include <string>

class SimpleHash
{
public:
    SimpleHash();
    ~SimpleHash() {}
    void Init(FreeNodePool* pool);
    
    bool insertword(std::string const& str);
    bool searchword(string const& str);
    node* deleteword(string const& str);
    void Print();
	
private:
    bool isPresent(const char* str, unsigned int hashval);
    unsigned int computeHash(const char* str);    //Using polynomial rolling hash
	
    unsigned int _simpleHashTable[MAX_WORD_COUNT/2];
    unsigned int _hashSize;
    FreeNodePool* _nodePool;
};

#endif
