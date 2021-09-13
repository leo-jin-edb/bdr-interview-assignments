#ifndef _DICTIONARY_H_
#define _DICTIONARY_H_

#include <string>
#include "nodepool.h"
#include "simplehash.h"

using namespace std;
#define MAX_CHAR 26

class SingleCharDictionary 
{
public:
	SingleCharDictionary();
	~SingleCharDictionary() { }
        void Init();
	
	bool insertword(node* tempNode);
	bool searchword(string const& str);
	node* deleteword(string const& str);
	void Print();
	
private:    
    sem_t _rwMutex;
	sem_t _rMutex;
	int   _readerCount; 
    SimpleHash _charHash;
};

class Dictionary
{
public:
	Dictionary() { _initialized = 0;}
	~Dictionary() { }
        void Init();
	
        bool insertword(string str);
	bool searchword(string str);
	bool deleteword(string str);
        void Print();
	
private:
		
    SingleCharDictionary _charDictionary[MAX_CHAR];
    FreeNodePool         _nodePool;
    int                  _initialized;
};

#endif
