//TODO: Documentation
#include "Os.h"


shared_memory_id os::shm_create(shared_memory_key key, size_t size, int flag)
{
    return ::shmget(key, size, IPC_CREAT | IPC_EXCL | flag);
}

shared_memory_id os::shm_open(shared_memory_key key, size_t size, int flag)
{
    return ::shmget(key, size, flag);
}
int os::shmctl(int shmid, int cmd)
{
    return ::shmctl(shmid, cmd, NULL);
}
int os::shm_remove(int shmid)
{
    return ::shmctl(shmid, IPC_RMID, NULL);
}

void*  os::shm_attach(shared_memory_id id, const void *ptr, int flag)
{
    return ::shmat(id, ptr, flag);
}

int os::shm_detach (void* addr)
{
    return ::shmdt((char*)addr);
}

int os::gettimeofday(struct timeval *tp)
{
    int retval = 0;
    retval = ::gettimeofday(tp, NULL);
    return retval;
}

int os::select(int nfds, fd_set *readfds, fd_set *writefds,
                 fd_set *exceptfds, struct timeval * timeout)
{
    return ::select(nfds, readfds, writefds, exceptfds, timeout);
}

struct tm* os::localtime(long *secs)
{
    return ::localtime(secs);
}


int os::sleep(int secs)
{ 
    return ::sleep(secs);
}
int os::usleep(int msecs)
{ 
    struct timeval timeout;
    timeout.tv_sec = 0;
    timeout.tv_usec = msecs;
    os::select(0,0,0,0, &timeout);
    return 0;
}
int os::getNoOfProcessors()
{
    return ::sysconf(_SC_NPROCESSORS_ONLN);
}
