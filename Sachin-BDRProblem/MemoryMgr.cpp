#include "MemoryMgr.hpp"

MemMgrMetaData	MemoryMgr::metaData		= nullptr;
BPtr			MemoryMgr::shmPtr		= nullptr;
bool			MemoryMgr::initialized	= false;

DicStatus MemoryMgr::InternalGetSharedMemory(DicConfig config) {

	DicStatus rc = DIC_SUCCESS;

	BOOST_TRY{

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

		cout << "Shared Memory address : " << shptr << endl;
	}
	BOOST_CATCH(interprocess_exception& ex) {

		cout << "Execption: " << ex.what() << endl;
		rc = BOOST_LIB_ERR;

	} BOOST_CATCH_END

	return rc;
}

DicStatus MemoryMgr::Initialize(DicConfig & config, const BPtr appdata, UInt32 appDataSize) {

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

			metaData->appDataSize	= appDataSize;
			metaData->freeOffset	= sizeof(MemMgrMetaData);

			// write app data.
			memcpy(shmPtr + metaData->freeOffset, appdata, appDataSize);
			metaData->freeOffset += appDataSize;
		
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

DicStatus	MemoryMgr::DeInitialize(bool Create, const string shareMemName, const BPtr appdata, UInt32 appDataSize) {

	if (!initialized)
		return MEM_INIT_ERROR;

	DicStatus rc = DIC_SUCCESS;

	BOOST_TRY {

		// TODO: Handle more scenarios of existing gracefully.
		scoped_lock<interprocess_mutex> lock(metaData->mutex);

		shared_memory_object::remove(config.shName);
	}
	BOOST_CATCH(interprocess_exception& ex) {

		cout << "Execption: " << ex.what() << endl;
		rc = BOOST_LIB_ERR;

	} BOOST_CATCH_END
		
	return rc;
}
	
DicStatus	MemoryMgr::GetAppData(BPtr & appData, UInt32 & appDataSize) {

	if (!initialized)
		return MEM_INIT_ERROR;

	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	appDataSize = metaData->appDataSize;
	memcpy(appData, shmPtr + sizeof(MemMgrMetaData), appDataSize);	

	return DIC_SUCCESS;
}
	
DicStatus MemoryMgr::AllocMem(BPtr & ptr, UInt32 size) {

	if (!initialized)
		return MEM_INIT_ERROR;

	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	if ((metaData->freeOffset + size) >= metaData->shmSize)
		return MEM_UNAVAILABLE;

	ptr = shmPtr + metaData->freeOffset;
	metaData->freeOffset += size;

	return DIC_SUCCESS;
}

