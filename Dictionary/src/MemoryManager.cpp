#include "Os.h"
#include  "MemoryManager.h"
//TODO: Documentation
RetVal MemoryManager::createSharedMemory()
{
	shared_memory_key key = Conf::config.getShmKey();
	m_shmId = os::shm_create(key, MAX_SM_SIZE, 0660);  
	if (-1 == m_shmId) 
	{
		if (errno == EEXIST)
		{
			printf ("Shared Memory already exists \n");
		}
		printf("Shared memory create failed\n");
		return ErrSysInternal; 
	}

	
	m_shmPt = os::shm_attach(m_shmId, NULL, SHM_RND);
	if (NULL == m_shmPt)
	{
		printf("Shared memory attach returned -ve value %d \n", m_shmPt);
		return ErrOS;
	}
	SharedMemoryMetadata *pSm = (SharedMemoryMetadata *) m_shmPt;
        pSm->m_mutex.getExclusiveLock();
	pSm -> m_totalNodeInUse = 0;
	pSm -> m_firstNodeOffset = SM_META_SIZE;
	pSm -> m_endNodeOffset = INVALID_OFFSET;
        pSm->m_mutex.releaseLock();

	return OK;
}

RetVal MemoryManager::deleteSharedMemory()
{
 	if (-1 != m_shmId)
	{
		os::shm_remove(m_shmId);
		m_shmId = -1;
		return OK;
	}
	else
	{
		return ErrSysInternal;
	}
}

RetVal MemoryManager::openSharedMemory()
{
	shared_memory_key key = Conf::config.getShmKey();
	m_shmId = os::shm_open(key, MAX_SM_SIZE, 0660);
	if (m_shmId == -1 )
	{
		printf("Shared memory open failed\n");
		return ErrOS;
	}

	m_shmPt = os::shm_attach(m_shmId, NULL, SHM_RND);
	if (NULL == m_shmPt)
	{
		printf("Shared memory attach returned -ve value %d\n", m_shmPt);
		return ErrOS;
	}
	return OK;
}

RetVal MemoryManager::closeSharedMemory()
{
	os::shm_detach(m_shmPt);
	return OK;
}

RetVal MemoryManager::allocate(WordNode **ppWord)
{
	RetVal ret = OK;
	if (m_shmPt == NULL)
	{
		ret = openSharedMemory();
		if (ret != OK)
		{
			printf("Shared memory attachment failed. Allocation failed!!!\n");
			return ret;
		}
	}
	SharedMemoryMetadata *pSm = (SharedMemoryMetadata *) m_shmPt;
        pSm->m_mutex.getShareLock();
	if (pSm->m_totalNodeInUse >= TOTAL_NO_WORD)
	{
		printf("Memory is full! Allocation failed!!!");
                pSm->m_mutex.releaseLock();
		return ErrNoMemory;
	}
	unsigned int offset = SM_META_SIZE;
	unsigned int offset1 = SM_META_SIZE;
        if (pSm->m_totalNodeInUse > 0)
        {
            offset = offset1 = pSm -> m_endNodeOffset;
        }
      
	// Check regular flow till end
	while (offset < MAX_SM_SIZE)
	{
		WordNode *pWord = (WordNode *)((char *)m_shmPt + offset);
                pWord->m_mutex.getShareLock();
		if (pWord->m_meta.m_isInUse == 0)
		{
                        pWord->m_mutex.getExclusiveLock(true);
			pWord->m_meta.m_isInUse = 1;
			pWord->m_meta.m_offset = offset;
			*ppWord = pWord;
			break;
		}
                pWord->m_mutex.releaseLock();
		offset += sizeof(WordNode);
	}
		
	if (NULL == (*ppWord))
	{
			
		//Check from begin
		unsigned int startOffset = SM_META_SIZE;
		while (startOffset < offset1)
		{
			WordNode *pWord = (WordNode *)((char *)m_shmPt + startOffset);
                        pWord->m_mutex.getShareLock();
			if (pWord->m_meta.m_isInUse == 0)
			{
                                pWord->m_mutex.getExclusiveLock(true);
				pWord->m_meta.m_isInUse = 1;
				pWord->m_meta.m_offset = startOffset;
				*ppWord = pWord;
				break;
			}
                        pWord->m_mutex.releaseLock();
			startOffset += sizeof(startOffset);
		}
	}
	
	if (*ppWord != NULL)
	{
                pSm->m_mutex.getExclusiveLock(true);
		pSm->m_totalNodeInUse++;
		pSm->m_endNodeOffset = (*ppWord)->m_meta.m_offset;

                printf (" Last offset %d New Offset %d\n", offset1, pSm->m_endNodeOffset);		

                if (pSm->m_totalNodeInUse > 1)
                {
			WordNode *pWord = (WordNode *)((char *)m_shmPt + offset1);                               
                	pWord->m_mutex.getExclusiveLock();
			pWord->m_meta.m_nextNodeOffset = (*ppWord)->m_meta.m_offset;
                	pWord->m_mutex.releaseLock();
		}
                (*ppWord)->m_meta.m_nextNodeOffset = INVALID_OFFSET; 
                (*ppWord)->m_mutex.releaseLock();

                pSm->m_mutex.releaseLock();
		return OK;
	}
	else
	{
		printf("Invalid memory link \n");
                pSm->m_mutex.releaseLock();
		return ErrSysFatal;
	}
}

RetVal MemoryManager::free(WordNode *pWord)
{
	RetVal ret = OK;
	if (m_shmPt == NULL)
	{
		printf("Shared memory is not attached!!!\n");
		return ErrSysInternal;
	}
	
	if (NULL == pWord)
	{
		printf("Invalid pointer to free!!!\n");
		return ErrSysInternal;
	}
	
	SharedMemoryMetadata *pSm = (SharedMemoryMetadata *) m_shmPt;
        pSm->m_mutex.getExclusiveLock();

	pSm->m_totalNodeInUse--;
        if (0 == pSm->m_totalNodeInUse)
        {
           pSm -> m_firstNodeOffset = SM_META_SIZE;
           pSm -> m_endNodeOffset = INVALID_OFFSET;
        }
	
        pWord->m_mutex.getExclusiveLock();
	pWord->m_meta.m_isInUse = 0;
	pWord->m_meta.m_offset = INVALID_OFFSET;
	pWord->m_meta.m_nextNodeOffset = INVALID_OFFSET;
        pWord->m_mutex.releaseLock();
	pSm->m_mutex.releaseLock();
	return OK;
}

RetVal MemoryManager::printStatistics()
{
	RetVal ret = OK;
        if (m_shmPt == NULL)
        {
                printf("Shared memory is not attached!!!\n");
                return ErrSysInternal;
        }
        SharedMemoryMetadata *pSm = (SharedMemoryMetadata *) m_shmPt;
        pSm->print();

        if (0 == pSm->m_totalNodeInUse)
        {
            printf("No data \n");
            return OK;
        }
        unsigned int offset = pSm->m_firstNodeOffset;
        //TODO: lock
        while (offset != INVALID_OFFSET)
        {
                WordNode *pWord = (WordNode *)((char *)pSm + offset);
                pWord->print ();
                offset = pWord->m_meta.m_nextNodeOffset;
        }
        return ret;
}
