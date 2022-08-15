#include "MemoryMgr.hpp"
#include <stdio.h>

MemoryMgr *			MemoryMgr::instance		= nullptr;




MemoryMgr::MemoryMgr(DicConfig *config) {


	shmPtr = InternalGetSharedMemory(config);

	Initialize(config);

};

MemoryMgr* MemoryMgr::Obj(DicConfig * config) {

	if (instance == nullptr) {

		if (config == nullptr)
			return nullptr;

		instance = new MemoryMgr(config);
	}

	return instance;
}


BPtr MemoryMgr::InternalGetSharedMemory(DicConfig * config) {

	BPtr shmAddr;

	cout << "Config: \n create/open: " << config->shCreate << endl;
	cout << "Name: " << config->shName << endl;
	cout << "Size: " << config->shSize << endl;

	if (config->shCreate) {

		// remove old SHM with same name.
		shared_memory_object::remove(config->shName);

		cout << "Creating Shared Memory" << endl;
		shm = new shared_memory_object(create_only, config->shName, read_write);
		shm->truncate(config->shSize); //Set size
	}
	else {

		cout << "Opening Shared Memory" << endl;
		shm = new shared_memory_object(open_only, config->shName, read_write);
	}

	//Map the whole shared memory in this process
	region = new mapped_region(*shm, read_write);

	shmAddr = (BPtr)region->get_address();

	if (shmAddr == nullptr)
		exit(EXIT_FAILURE); // possibly due to race condition. 2 parallel processes trying to create 

	if (shmAddr == nullptr) {
		cout << "SHM Creation Failed." << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Shared Memory address : " << (UInt32)shmAddr << endl;

	return shmAddr;
}

void MemoryMgr::Initialize(DicConfig * config) {

	if (config->shCreate) {

		// allocate MemMgrMetaData
		metaData = new (shmPtr) MemMgrMetaData;

		// allocate MemMgrMetaData
		scoped_lock<interprocess_mutex> lock(metaData->mutex);

		metaData->shmSize		= config->shSize;
		memcpy(metaData->shName, config->shName, SHM_NAME_SIZE);

		metaData->freeOffset	= sizeof(MemMgrMetaData);

		cout << "MemMgr current offset: " << metaData->freeOffset << endl;
	}
	else {

		// allocate MemMgrMetaData
		metaData =  (MemMgrMetaData *)shmPtr;

		// allocate MemMgrMetaData
		scoped_lock<interprocess_mutex> lock(metaData->mutex);

		if (strncmp(metaData->shName, config->shName, SHM_NAME_SIZE) != 0)
			exit(EXIT_FAILURE);
		
		cout << "MemMgr current offset: " << metaData->freeOffset << endl;
	}

}

DicStatus	MemoryMgr::DeInitialize() {

	DicStatus rc = DIC_SUCCESS;
	
	// TODO: what is someone is using SHM currently? Handle graceful shutdown.
	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	shared_memory_object::remove(metaData->shName);

	return rc;
}	
	
BPtr MemoryMgr::AllocMem(UInt32 size) {

	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	if ((metaData->freeOffset + size) >= metaData->shmSize)
		return nullptr;

	BPtr ptr = shmPtr + metaData->freeOffset;
	metaData->freeOffset += size;

	cout << "MemMgr: ptr: " << ptr << ". New offset: " << metaData->freeOffset << endl;

	return ptr;
}

BPtr MemoryMgr::GetAppDataBuff(UInt32 offset) {


	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	cout << "MemMgr: AppData Ptr: " << BPtr(shmPtr +  sizeof(MemMgrMetaData) + offset) << endl;

	return shmPtr +  sizeof(MemMgrMetaData) + offset;
}