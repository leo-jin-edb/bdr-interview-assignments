#include "nodepool.h"
#include <unistd.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
                        } while (0)

FreeNodePool::FreeNodePool()
{  

}

FreeNodePool::~FreeNodePool()
{

}

void FreeNodePool::Init()
{
    for(int i = 0; i < MAX_WORD_COUNT-1; i++)
    {
        _nodePool[i]._index = i;
        _nodePool[i]._next = i+1;
    }
    _freeNode = 0;

    if (sem_init(&_freeNodePoolSem, 1, 1) == -1)
        errExit("sem_init-sem1");
}

node* FreeNodePool::GetFreeNode()
{
    sem_wait(&_freeNodePoolSem);
    
    if(_freeNode == ~0)
    {
    	sem_post(&_freeNodePoolSem);
        return 0;
    }
    node* n = &_nodePool[_freeNode];
    _freeNode = _nodePool[_freeNode]._next;
    n->_next = ~0;
    
    sem_post(&_freeNodePoolSem);
    return n;
}
	
node* FreeNodePool::GetNode(unsigned int index)
{
     if(index == ~0) {
        return 0;
     }
    return &_nodePool[index];
}

void FreeNodePool::ReleaseFreeNode(node* n)
{
    sem_wait(&_freeNodePoolSem);
    n->_next = _freeNode;
    _freeNode = n->_index;
    sem_post(&_freeNodePoolSem);
}
