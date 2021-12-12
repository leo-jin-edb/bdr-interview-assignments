#include "Os.h"
#include "Mutex.h"
#include "Config.h"
//TODO: Documentation

Mutex::Mutex()
{
#ifdef CUSTOM_LOCK
    lock = 0;
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mutex_, &attr);
#endif
}

int Mutex::init()
{
#ifdef CUSTOM_LOCK
    lock = 0;
#else
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    int ret = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&mutex_, &attr);
    pthread_mutexattr_destroy(&attr);
#endif
    return 0;
}

int Mutex::tryShareLock(int tryTimes, int waitmsecs,bool share, bool upgrade)
{
    int ret = 0;
#ifdef CUSTOM_LOCK
    int oldValue = (int)lock;
    if (oldValue >= 0  && share)
    {
        ret = CAS((int*)&lock, oldValue, oldValue+1);
    }
    else if ((oldValue == 1 && upgrade ) || ( !share && oldValue == 0))
    {
        ret = CAS((int*)&lock, oldValue, -1);
    }
    else 
    { 
      ret = 1;
    } 
    if (0 == ret) 
        return 0; 
#endif
    int tries = 1;
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = waitmsecs;
    if (tryTimes == 0 && waitmsecs == 0)
    {
        timeout.tv_sec = Conf::config.getMutexSecs();
        timeout.tv_usec = Conf::config.getMutexUSecs();
        tryTimes = Conf::config.getMutexRetries();
    }
    int cnt=0;
    while (tries < tryTimes)
    {
#ifdef CUSTOM_LOCK
    if (os::getNoOfProcessors() >1) {
        cnt=0;
        while(true) {
            oldValue = (int)lock;
            if (oldValue >= 0  && share) {
                ret = CAS((int*)&lock, oldValue, oldValue+1);
            }else if ((oldValue == 1 && upgrade ) || (!share && oldValue == 0) ) {
                ret = CAS((int*)&lock, oldValue, -1);
            }else { ret = 1; }
         
            if(0 == ret ) return 0;
            cnt++;
            if (cnt == tryTimes * 100) break;
         }
    }else {
            oldValue = (int)lock;
            if (oldValue >= 0  && share) {
                ret = CAS((int*)&lock, oldValue, oldValue+1);
            }else if ((oldValue == 1 && upgrade ) || (!share && oldValue == 0) ){
                ret = CAS((int*)&lock, oldValue, -1);
            } else { ret =1;}
            if(ret==0) return 0;
    }
#else
    ret = pthread_mutex_trylock(&mutex_);
    if (EBUSY  != ret) 
        return 0;

#endif
        os::select(0, 0, 0, 0, &timeout);
        tries++;
    }
    printf("Unable to get the mutex tried %d times\n", tries);
    return 1;
}

int Mutex::getShareLock()
{
    int ret=0;
#ifdef CUSTOM_LOCK
    ret = tryShareLock(0, 0, true, false);
    return ret;
#else
        //printf ("Mutex taken %p\n", this);
    ret = pthread_mutex_lock(&mutex_);
#endif
    if (ret == 0) 
        return 0;
    else
        return 1;
}

int Mutex::getExclusiveLock(bool upgrade)
{
    int ret=0;
#ifdef CUSTOM_LOCK
    ret = tryShareLock(0, 0, false, upgrade);
    return ret;
#else
        if (!upgrade)
    {
                //printf ("Mutex taken %p\n", this);
        ret = pthread_mutex_lock(&mutex_);
    }
#endif
    if (ret == 0) 
        return 0;
    else
        return 1;
}

int Mutex::releaseLock()
{
    int ret=0;
#ifdef CUSTOM_LOCK
    int oldValue  = (int)lock;
    if( oldValue > 1)
    { 
        ret = CAS((int*)&lock, oldValue, oldValue-1 );
    }
    if( oldValue == 1 || oldValue == -1 )
    {
        ret = CAS((int*)&lock, oldValue, 0 );   
    }
    if( ret != 0)
    {
        int tries = 1;
        struct timeval timeout;
        timeout.tv_sec = Conf::config.getMutexSecs();
        timeout.tv_usec = Conf::config.getMutexUSecs();
        int tryTimes = Conf::config.getMutexRetries();
        while (tries < tryTimes)
        {
             oldValue = (int)lock;
             if( oldValue > 1){
                 ret = CAS((int*)&lock, oldValue, (*(int*)&lock)-1 );
                 if(ret == 0) break;
             }
             if( oldValue == 1 || oldValue == -1 )
             {
                  ret = CAS((int*)&lock, oldValue, 0 );
                  if(ret == 0) break;
             }
             tries++; 
        }
    }
#else
        //printf ("Mutex release %p\n", this);
    ret = pthread_mutex_unlock(&mutex_);
#endif
    if (ret == 0) 
        return 0;
    else
        return 1;
}

int Mutex::destroy()
{
#ifdef CUSTOM_LOCK
#else
    return pthread_mutex_destroy(&mutex_);
#endif
    return 0;
}

int Mutex::recoverMutex()
{
    int ret=0;
#ifdef CUSTOM_LOCK
    lock = 0;
#else
    ret = pthread_mutex_unlock(&mutex_);
#endif
    return ret;
}
void Mutex::print()
{
#ifdef CUSTOM_LOCK
    printf("Value of Lock %d\n", lock);
#endif
}
int Mutex::CAS(int *ptr, int oldVal, int newVal)
{
#ifdef CUSTOM_LOCK
        unsigned char ret;
        __asm__ __volatile__ (
                "  lock\n"
                "  cmpxchgl %2,%1\n"
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (newVal), "m" (*ptr), "a" (oldVal)
                : "memory");

        //above assembly returns 0 in case of failure
        if (ret) return 0;

        struct timeval timeout;
        timeout.tv_sec=0;
        timeout.tv_usec=1000;
        os::select(0,0,0,0, &timeout);
            __asm__ __volatile__ (
                "  lock\n"
                "  cmpxchgl %2,%1\n"
                "  sete %0\n"
                : "=q" (ret), "=m" (*ptr)
                : "r" (newVal), "m" (*ptr), "a" (oldVal)
                : "memory");
        //if (ret) return 0;  else {printf("CAS Fails %d-\n", ret); return 1; }
        if (ret) return 0;  else return 1;
#else
    return 0;
#endif
}
