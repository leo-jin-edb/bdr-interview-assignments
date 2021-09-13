#include "simplehash.h"
#include <unistd.h>
#include <iostream>
#include <ostream>
#include <string>
#include <cstring>

using namespace std;

SimpleHash::SimpleHash()
{
    _hashSize = MAX_WORD_COUNT/2;
}

unsigned int SimpleHash::computeHash(const char* str)
{
    unsigned int hashval = 0;

    for (int i = 0; str[i] != '\0'; i++)
        hashval = str[i] + 31*hashval;
		
    return hashval % _hashSize;
} 

bool SimpleHash::insertword(node* tempNode)
{
    unsigned int hashval = computeHash(tempNode->_word);
    if(!isPresent(tempNode->_word, hashval))
    {
        cout << " str " << tempNode->_word << " hashval " << hashval << " is inserting" << endl;
    	tempNode->_next = _simpleHashTable[hashval];
    	
        _simpleHashTable[hashval] = tempNode;
    	return true;
    }
    return false;
}

bool SimpleHash::searchword(std::string const& str)
{
    unsigned int hashval = computeHash(str.c_str());
    return isPresent(str.c_str(), hashval);
}

bool SimpleHash::isPresent(const char* str, unsigned int hashval)
{
    node* tempNode = _simpleHashTable[hashval];
    while(tempNode)
    {
        if( !strcmp(str, tempNode->_word))
        {
            cout << " str " << str << " hashval " << hashval << " is present" << endl;
    	    return true;
        }
    	tempNode = tempNode->_next;
    }
    cout << " str " << str << " hashval " << hashval << " is not present" << endl;
    return false;
}

node* SimpleHash::deleteword(std::string const& str)
{
    unsigned int hashval = computeHash(str.c_str());
    node* prevNode = 0;
    node* tempNode = _simpleHashTable[hashval];
    
    while(tempNode)
    {
        if(!str.compare(tempNode->_word))
    	{
    	    if(prevNode)
            {
                cout << " str " << tempNode->_word << " hashval " << hashval << " is deleting" << endl;
                prevNode->_next = tempNode->_next;
            }
            else
            {
                cout << " str " << tempNode->_word << " hashval " << hashval << " is deleting" << endl;
                _simpleHashTable[hashval] = 0;
            }
            return tempNode;
        }
        prevNode = tempNode;
        tempNode = tempNode->_next;
    }
    cout << " str " << tempNode->_word << " hashval " << hashval << " is not able to delete" << endl;
    return 0;
}

void SimpleHash::Print()
{
    for(int i = 0; i < MAX_WORD_COUNT/2; i++)
    {
        node* tempNode = _simpleHashTable[i];
        while(tempNode){
            cout << "Index " << i << " - " << tempNode->_word << endl;
            tempNode = tempNode->_next;
        }
    }
}
