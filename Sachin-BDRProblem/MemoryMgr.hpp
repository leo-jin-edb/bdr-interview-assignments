#pragma once

#include "Basics.hpp"

void mutex_init(pthread_mutex_t *mutex);

struct MemMgrMetaData {

	MemMgrMetaData(){}

	pthread_mutex_t 	mutex;
	Int8				shName[SHM_NAME_SIZE];
	UInt32				shmSize;
	UInt32				freeOffset;					// start of free memory.
};

class MemoryMgr {

public:

	static MemoryMgr * 	Obj(DicConfig *config = nullptr);
	static DicStatus	DeInitialize();
	
	// Internal functions.
	VPtr 		InternalGetSharedMemory_Posix(DicConfig *config);
	// APIs
	void		Initialize(DicConfig * config);

	UInt32		AllocMem(UInt32 size);
	VPtr		GetAppDataBuff(UInt32 offset);

	VPtr		Ptr(UInt32 offset);

private:

	static MemoryMgr*		instance;

	// Defined private in order to prevent instantiating this class as all members are used as static.
	MemoryMgr(DicConfig *config);

	MemMgrMetaData *		metaData;
	VPtr					shmPtr;
};

