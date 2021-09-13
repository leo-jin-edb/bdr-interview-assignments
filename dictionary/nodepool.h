#ifndef _NODE_POOL_H_
#define _NODE_POOL_H_

#include <semaphore.h>
#include <string>

using namespace std;

#define MAX_WORD_COUNT 1000000 
#define MAX_WORD_SIZE 20

struct node
{
    node()
    {
        _next = 0;
    }
    
    char _word[MAX_WORD_SIZE];
    struct node* _next;
};

class FreeNodePool
{

public:
    FreeNodePool(); 
    ~FreeNodePool();
    void Init();
    
    node* GetFreeNode();
    void ReleaseFreeNode(node* n);
	
private:
    sem_t _freeNodePoolSem;
    node* _freeNode;
    node _nodePool[MAX_WORD_COUNT];
};

#endif
