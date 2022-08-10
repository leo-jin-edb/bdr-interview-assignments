#include "MemoryMgr.hpp"

MemMgrMetaData *	MemoryMgr::metaData		= nullptr;
BPtr				MemoryMgr::shmPtr		= nullptr;
bool				MemoryMgr::initialized	= false;

DicStatus MemoryMgr::InternalGetSharedMemory(DicConfig config) {

	DicStatus rc = DIC_SUCCESS;

	shared_memory_object * shm;

	if (config.shCreate) {

		// remove old SHM with same name.
		shared_memory_object::remove(config.shName);

		cout << "Creating Shared Memory" << endl;
		shm = new shared_memory_object(create_only, config.shName, read_write);
		shm->truncate(config.shSize); //Set size
	}
	else {

		cout << "Opening Shared Memory" << endl;
		shm = new shared_memory_object(open_only, config.shName, read_write);
	}

	//Map the whole shared memory in this process
	mapped_region region(*shm , read_write);

	shmPtr = (BPtr)region.get_address();
	if (shmPtr == nullptr)
		return MEM_INIT_ERROR;

	cout << "Shared Memory address : " << shmPtr << endl;
	

	return rc;
}

DicStatus MemoryMgr::Initialize(DicConfig & config) {

	// Ip params are already validated.

	if (initialized)
		return MEM_INIT_ERROR;

	DicStatus rc; 

	rc = InternalGetSharedMemory(config);

	// if no error so far, we are safe to proceed to init SHM. No one else will try to initialize now.
	if (!rc) {

		if (config.shCreate) {

			// allocate MemMgrMetaData
			metaData = new (shmPtr) MemMgrMetaData;

			// allocate MemMgrMetaData
			scoped_lock<interprocess_mutex> lock(metaData->mutex);
			cout << "MemoryMgr: Mutex locked for CreateMgr." << endl;

			metaData->shmSize		= config.shSize;
			memcpy(metaData->shName, config.shName, SHM_NAME_SIZE);

			metaData->freeOffset	= sizeof(MemMgrMetaData);
		
			initialized = true;
		}
		else {

			// allocate MemMgrMetaData
			metaData =  (MemMgrMetaData *)shmPtr;

			// allocate MemMgrMetaData
			scoped_lock<interprocess_mutex> lock(metaData->mutex);
			cout << "MemoryMgr: Mutex locked for OpenMgr." << endl;

			if (initialized == false || strncmp(metaData->shName, config.shName, SHM_NAME_SIZE) != 0)
				rc = MEM_INIT_ERROR;
		}

		cout << "MemoryMgr Mutex UnLocked." << endl;
	}

	return rc;
}

DicStatus	MemoryMgr::DeInitialize() {

	if (!initialized)
		return MEM_INIT_ERROR;

	DicStatus rc = DIC_SUCCESS;
	
	// TODO: what is someone is using SHM currently? Handle graceful shutdown.
	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	shared_memory_object::remove(metaData->shName);
	
		
	return rc;
}	
	
BPtr MemoryMgr::AllocMem(UInt32 size) {

	if (!initialized)
		return nullptr;

	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	if ((metaData->freeOffset + size) >= metaData->shmSize)
		return nullptr;

	BPtr ptr = shmPtr + metaData->freeOffset;
	metaData->freeOffset += size;

	return ptr;
}

