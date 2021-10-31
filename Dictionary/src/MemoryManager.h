#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "Common.h"
#include "Config.h"

class MemoryManager
{
	
	void *m_shmPt;
	shared_memory_id m_shmId;
	
	public:
	MemoryManager()
	{
		m_shmPt = NULL;
		m_shmId = -1;
	}
	

	inline void *getShmPtr() { return m_shmPt;}
	RetVal createSharedMemory();
	RetVal openSharedMemory();
	RetVal closeSharedMemory();
    	RetVal deleteSharedMemory();
	RetVal allocate(WordNode **ppWord);
	RetVal free(WordNode *pWord);
	RetVal printStatistics();

};

#endif
