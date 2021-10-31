#ifndef OS_H
#define OS_H

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>

#include <sys/shm.h>
#include <sys/errno.h>
#include <sys/time.h>


typedef key_t shared_memory_key;
typedef int   shared_memory_id;

class  os
{
	public:
   
	static int usleep(int microsecs);
	static int sleep(int secs);

	static shared_memory_id shm_create(shared_memory_key key, size_t size, int flag);
	static shared_memory_id shm_open(shared_memory_key key, size_t size, int flag);
	static void*  shm_attach(shared_memory_id id, const void *ptr, int flag);
	static int shm_detach (void*);
	static int shmctl(int shmid, int cmd);
	static int shm_remove(int shmid);
	
	static int select(int nfds, fd_set *readfds, fd_set *writefds,
					  fd_set *exceptfds, struct timeval * timeout);
	static int gettimeofday(struct timeval *tp);
	static struct tm* localtime(long *secs);
        static int getNoOfProcessors();
	
};

#endif
