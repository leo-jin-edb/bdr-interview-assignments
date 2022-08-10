#pragma once

#include "Basics.hpp"

struct MemMgrMetaData {

	interprocess_mutex 	mutex;						// from boost library.
	char				shName[SHM_NAME_SIZE];
	UInt32				shmSize;
	UInt32				appDataSize;
	UInt32				freeOffset;					// start of free memory.
};

class MemoryMgr {

public:
	
	// Internal functions.
	static DicStatus	InternalGetSharedMemory(DicConfig config);

	// APIs
	static DicStatus	Initialize(DicConfig& config, const BPtr appdata, UInt32 appDataSize);
	static DicStatus	DeInitialize(bool Create, const string shareMemName, const BPtr appdata, UInt32 appDataSize);

	static DicStatus	GetAppData(BPtr& appData, UInt32& appDataSize);

	static DicStatus	AllocMem(BPtr& ptr, UInt32 size);

private:

	// Defined private in order to prevent instantiating this class as all members are used as static.
	MemoryMgr();

	static MemMgrMetaData*		metaData;
	static BPtr					shmPtr;
	static bool					initialized;
};

