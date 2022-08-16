#pragma once

#include "Basics.hpp"

struct MemMgrMetaData {

	MemMgrMetaData(){}

	interprocess_mutex 	mutex;						// from boost library.
	Int8				shName[SHM_NAME_SIZE];
	UInt32				shmSize;
	UInt32				freeOffset;					// start of free memory.
};

class MemoryMgr {

public:

	static MemoryMgr * Obj(DicConfig *config = nullptr);
	
	// Internal functions.
	VPtr 		InternalGetSharedMemory_Posix(DicConfig *config);
	VPtr		InternalGetSharedMemory(DicConfig * config);

	// APIs
	void		Initialize(DicConfig * config);
	DicStatus	DeInitialize();

	UInt32		AllocMem(UInt32 size);
	VPtr		GetAppDataBuff(UInt32 offset);

	VPtr		Ptr(UInt32 offset);

private:

	static MemoryMgr*		instance;

	// Defined private in order to prevent instantiating this class as all members are used as static.
	MemoryMgr(DicConfig *config);

	// TODO: ~MemoryMgr(BPtr shm);  call DeInitialize, remove SHM

	MemMgrMetaData *		metaData;
	VPtr					shmPtr;

	// boost shared memory (alternate option)
	shared_memory_object * 	shm;
	mapped_region *			region;
};

