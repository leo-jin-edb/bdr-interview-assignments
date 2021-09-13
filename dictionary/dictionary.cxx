#include "dictionary.h"
#include <algorithm>    // std::transform:
#include <cstring>

using namespace std;

SingleCharDictionary::SingleCharDictionary()
{

}

void SingleCharDictionary::Init()
{
    sem_init(&_rwMutex, 1, 1);
    sem_init(&_rMutex, 1, 1);
}

bool SingleCharDictionary::insertword(node* tempNode)
{
    bool ret;
	
    sem_wait(&_rwMutex);
	ret = _charHash.insertword(tempNode);
	sem_post(&_rwMutex);
	
	return ret;
}

bool SingleCharDictionary::searchword(std::string const&str)
{
    bool ret;
	
	sem_wait(&_rMutex);
	_readerCount++;
	
	if(_readerCount == 1)
	{
	    sem_wait(&_rwMutex);
	}
	
	sem_post(&_rMutex);
	
	ret = _charHash.searchword(str);
	
	sem_wait(&_rMutex);
	_readerCount--;
	
	if(_readerCount == 0)
	{
	    sem_post(&_rwMutex);
	}
	
	sem_post(&_rMutex);
	return ret;
}

node* SingleCharDictionary::deleteword(std::string const&str)
{
    node* tempNode;
	
    sem_wait(&_rwMutex);
	tempNode = _charHash.deleteword(str);
	sem_post(&_rwMutex);
	
	return tempNode;
}

void SingleCharDictionary::Print()
{
	sem_wait(&_rwMutex);
	_charHash.Print();
	sem_post(&_rwMutex);
}

void Dictionary::Init()
{
    _nodePool.Init();
    for(int i = 0; i < MAX_CHAR; i++)
    {
        _charDictionary[i].Init();
    }
    _initialized = 101;
    while(1);
}

bool Dictionary::insertword(std::string str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	
	int index = str[0] - 97;
	
	node* tempNode = _nodePool.GetFreeNode();
	strcpy(tempNode->_word, str.c_str());
	tempNode->_next = 0;

	return _charDictionary[index].insertword(tempNode);
    
}

bool Dictionary::searchword(std::string str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	
	int index = str[0] - 97;
	return _charDictionary[index].searchword(str);
}

bool Dictionary::deleteword(std::string str)
{
	transform(str.begin(), str.end(), str.begin(), ::tolower);
	
	int index = str[0] - 97;
	node* tempNode = _charDictionary[index].deleteword(str);
	if(tempNode)
	{
	    _nodePool.ReleaseFreeNode(tempNode);
            return true;
	}
	
	return false;
}

void Dictionary::Print()
{
    for(int i = 0; i < MAX_CHAR; i++)
	{
	    _charDictionary[i].Print();
	}
}
