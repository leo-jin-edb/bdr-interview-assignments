#include "WordManager.h"
//TODO: Documentation
WordManager::WordManager()
{
	m_mManager.openSharedMemory();
}

WordManager::~WordManager()
{
	m_mManager.closeSharedMemory();
}

RetVal WordManager::insertWord(char *word)
{
	WordNode *pWord = NULL;
	RetVal ret = searchWord(word);
        if (ret != OK)
        {
		ret = m_mManager.allocate(&pWord);
		if (ret != OK)
		{
			return ret;
		}
	
		if(pWord != NULL)
		{
			pWord->m_mutex.getExclusiveLock();
			strncpy (pWord->m_data, word, (WORD_LENGTH_LENGTH - 1)); 
			pWord->m_mutex.releaseLock();
		}
                return OK;
	}
        else
        {
		return ErrAlready;
	}
}
//TODO: convert to index search
RetVal WordManager::searchWord(char *word)
{
	SharedMemoryMetadata *pSm = (SharedMemoryMetadata *) m_mManager.getShmPtr();
	bool isFound = false;
	if (NULL == pSm)
	{
		printf("Share memory has not created\n");
		return ErrOS;
	}
        pSm->m_mutex.getShareLock();
	if (0 == pSm->m_totalNodeInUse)
	{
		printf ("No data found \n");
                pSm->m_mutex.releaseLock();
		return ErrNotExists;
	}
	unsigned int offset = pSm->m_firstNodeOffset;
        pSm->m_mutex.releaseLock();

	while (offset != INVALID_OFFSET)
	{
		WordNode *pWord = (WordNode *)((char *)pSm + offset);
                pWord->m_mutex.getShareLock();
		if (strcmp(pWord->m_data, word) == 0)
		{
			isFound = true;
                        pWord->m_mutex.releaseLock();
			break;
		}
		offset = pWord->m_meta.m_nextNodeOffset;
                pWord->m_mutex.releaseLock();
	}
    
	if (isFound)
	{
		return OK;
	}
	else
	{
		return ErrNotExists;
	}
}

RetVal WordManager::deleteWord(char *word)
{
	SharedMemoryMetadata *pSm = (SharedMemoryMetadata *) m_mManager.getShmPtr();
	bool isFound = false;
	if (NULL == pSm)
	{
		printf("Share memory has not created\n");
		return ErrSysInternal;
	}

        pSm->m_mutex.getShareLock();
	unsigned int offset = pSm->m_firstNodeOffset;
	WordNode *pWord = NULL;
	WordNode *pPrevWord = NULL;

	//TODO: lock
	while (offset != INVALID_OFFSET)
	{
		pWord = (WordNode *)((char *)pSm + offset);
                pWord->m_mutex.getShareLock();
		if (strcmp(pWord->m_data, word) == 0)
		{
			isFound = true;
			break;
		}
		pPrevWord = pWord;
		offset = pWord->m_meta.m_nextNodeOffset;
                pWord->m_mutex.releaseLock();
	}
	if (isFound)
	{
		if(pPrevWord != NULL)
		{
                        pPrevWord->m_mutex.getExclusiveLock();
			pPrevWord->m_meta.m_nextNodeOffset = pWord->m_meta.m_nextNodeOffset;
                        pPrevWord->m_mutex.releaseLock();
		}
		else
		{
		        pSm->m_mutex.getExclusiveLock(true);
                        if (pSm -> m_totalNodeInUse > 1)
			{                       
				pSm->m_firstNodeOffset = pWord->m_meta.m_nextNodeOffset;
			}
		}
                pWord->m_mutex.releaseLock();
		pSm->m_mutex.releaseLock();
		return m_mManager.free(pWord);
	}
	else
	{
		printf ("Word is not exists\n");
        	pSm->m_mutex.releaseLock();
		return ErrNotExists;
	}
}
