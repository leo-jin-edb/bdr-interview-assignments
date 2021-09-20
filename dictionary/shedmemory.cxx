#include "shedmemory.h"
#include <sys/mman.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#define SHMPATH "inmemdict"

ShedMemory::ShedMemory(unsigned long size)
: _shmFD(-1)
, _shmPtr(0)
{
    _shmMemSize = size;
}

char* ShedMemory::GetShedMemory(bool &alreadyExist)
{	
    /* Create shared memory object and set its size to the size
      of our structure. */
    bool test = false;
    alreadyExist = false;
    if(test)
        shm_unlink(SHMPATH);

    _shmFD = shm_open(SHMPATH, O_RDWR, S_IRUSR | S_IWUSR);
    if(_shmFD != -1)
    {
        alreadyExist = true;   
    } else {
        _shmFD = shm_open(SHMPATH, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
        if (_shmFD == -1)
        {
            perror("Shared memory open failed.");
            exit(EXIT_FAILURE);
        }

        if (ftruncate(_shmFD, _shmMemSize) == -1)
        {
            perror("Shared memory truncate failed.");
            exit(EXIT_FAILURE);
        }
    }
    						 
     _shmPtr = (char*)mmap(NULL, _shmMemSize,PROT_READ | PROT_WRITE, MAP_SHARED, _shmFD, 0);
    // std::cout << "_shmPtr " << std::hex << (unsigned long)_shmPtr << std::endl;
     return _shmPtr;

}

void ShedMemory::ReleaseShedMemory()
{
    /* Unlink the shared memory object. Even if the peer process
      is still using the object, this is okay. The object will
      be removed only after all open references are closed. */
    
    shm_unlink(SHMPATH);
}


