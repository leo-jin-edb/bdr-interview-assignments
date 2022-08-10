#pragma once

#include "Basics.hpp"

struct MemMgrMetaData {

	interprocess_mutex 	mutex;						// from boost library.
	Int8				shName[SHM_NAME_SIZE];
	UInt32				shmSize;
	UInt32				freeOffset;					// start of free memory.
};

class MemoryMgr {

public:
	
	// Internal functions.
	static DicStatus	InternalGetSharedMemory(DicConfig config);

	// APIs
	static DicStatus	Initialize(DicConfig& config);
	static DicStatus	DeInitialize();

	static BPtr			AllocMem(UInt32 size);

private:

	// Defined private in order to prevent instantiating this class as all members are used as static.
	MemoryMgr();

	static MemMgrMetaData*		metaData;
	static BPtr					shmPtr;
	static bool					initialized;
};

