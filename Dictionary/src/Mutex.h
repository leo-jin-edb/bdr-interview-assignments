#ifndef MUTEX_H
#define MUTEX_H
#include "Os.h"

typedef int Lock;
typedef int IntUse;

class  Mutex
{

    #ifdef CUSTOM_LOCK
    Lock lock;
    #else
    pthread_mutex_t mutex_;
    #endif

    public:

    Mutex();
    int init();

    int tryShareLock(int tries=0, int waitmsecs=0,bool share=false,bool upgrade=false);

    int getShareLock ();

    int getExclusiveLock (bool upgrade=false);

    int releaseLock ();

    int destroy();

    int recoverMutex();

    static int CAS(int *ptr, int oldVal, int newVal);

};

#endif
