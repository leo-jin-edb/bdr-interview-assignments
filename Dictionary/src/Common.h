#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <string.h>
#include "Mutex.h"


#define WORD_LENGTH_LENGTH 256
#define TOTAL_NO_WORD  1000000
#define INVALID_OFFSET -1

struct SharedMemoryMetadata
{
	unsigned int m_totalNodeInUse;
	unsigned int m_firstNodeOffset;
	unsigned int m_endNodeOffset;
	Mutex m_mutex;
	SharedMemoryMetadata()
	{
		clear();
	}
	
	void clear()
	{
		m_totalNodeInUse = 0;
		m_firstNodeOffset = INVALID_OFFSET;
		m_endNodeOffset = INVALID_OFFSET;		
	}
	void print()
	{
		printf ("SM Metadata:  m_totalNodeInUse %d, m_firstNodeOffset %d m_endNodeOffset %d\n", m_totalNodeInUse, m_firstNodeOffset, m_endNodeOffset);
#ifdef CUSTOM_LOCK
		m_mutex.print();
#endif
	}
};

struct WordNodeMetadate
{
	bool m_isInUse;
	int m_offset;
	int m_nextNodeOffset;
	
	WordNodeMetadate()
	{
		clear();
	}
	
	void clear()
	{
	   	m_isInUse = false;
		m_offset = INVALID_OFFSET;
		m_nextNodeOffset = INVALID_OFFSET;
	}
};

struct WordNode
{
	WordNodeMetadate m_meta;
	char m_data [WORD_LENGTH_LENGTH];
	Mutex m_mutex;
	WordNode()
	{
		m_meta.clear();
		memset(m_data, '0', WORD_LENGTH_LENGTH);
	}
	
	void print()
	{
		printf ("Metadata: isInUse %d, offset %d nextOffSet %d \n", 
				 m_meta.m_isInUse, m_meta.m_offset, m_meta.m_nextNodeOffset);
		printf ("Data:  %s \n",  m_data);
#ifdef CUSTOM_LOCK
		m_mutex.print();
#endif
	}
};	


#define MAX_SM_SIZE  (sizeof(SharedMemoryMetadata) + (TOTAL_NO_WORD*sizeof(WordNode)))
#define SM_META_SIZE sizeof(SharedMemoryMetadata) 

/**
 * ErrCode enum
 */
enum RetVal
{
	OK		=  0, /**< OK. No error. Operation succeded. */
	ErrSysFatal	= -1, 
	ErrSysInternal	= -2,
	ErrNotExists	= -3,
	ErrNoMemory	= -4,
	ErrAlready	= -5,
	ErrOS		= -6,
	ErrUnknown	= -7
};


/**
 * Command prompt input option.
 */
enum InputOption
{
	SEARCH_OPT	= 0,
	INSERT_OPT	= 1,
	DELETE_OPT	= 2,
	END_OPT		= 3
};

#define TOTAL_ARGS 3
#endif
