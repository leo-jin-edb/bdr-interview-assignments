#ifndef _SHED_MEMORY_H_
#define _SHED_MEMORY_H_

#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */

/*
Class to create, map and release shared memory.
*/
class ShedMemory
{
public:
    ShedMemory(unsigned long size);
    ~ShedMemory() { }
    
    char* GetShedMemory(bool &alreadyExist);
    void  ReleaseShedMemory();
	
private:
    int            _shmFD;
    char*          _shmPtr;
    char	   _shmName[20];
    unsigned long  _shmMemSize;
};


#endif
