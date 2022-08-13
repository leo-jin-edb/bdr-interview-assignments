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
	BPtr		InternalGetSharedMemory(DicConfig * config);

	// APIs
	void		Initialize(DicConfig * config);
	DicStatus	DeInitialize();

	BPtr		AllocMem(UInt32 size);

private:

	static MemoryMgr*		instance;

	// Defined private in order to prevent instantiating this class as all members are used as static.
	MemoryMgr(DicConfig *config);
	// TODO: ~MemoryMgr(BPtr shm);  call DeInitialize

	MemMgrMetaData *		metaData;
	BPtr					shmPtr;
	shared_memory_object * 	shm;
	mapped_region *			region;
};

