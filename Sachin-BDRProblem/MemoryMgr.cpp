#include "MemoryMgr.hpp"
#include <stdio.h>

MemoryMgr *			MemoryMgr::instance		= nullptr;

void print(VPtr addr, DicConfig * config) {


	cout << "Complete Data: " << endl;
	for (int i = 0; i < 0; i++) {
		cout << ((BPtr)addr + i)[0];
	}

	cout << "Config: \n create/open: " << config->shCreate << endl;
	cout << "Name: " << config->shName << endl;
	cout << "Size: " << config->shSize << endl;

	cout << "\nShared Memory address : " << (UInt32 *)addr << endl;

}

MemoryMgr::MemoryMgr(DicConfig *config) {


	shmPtr = InternalGetSharedMemory_Posix(config);

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
VPtr MemoryMgr::InternalGetSharedMemory_Posix(DicConfig * config) {

	VPtr shmAddr;

	if (config->shCreate) {

		int shmid =shmget((key_t)2345, config->shSize, 0666|IPC_CREAT); 
		
		printf("Key of shared memory is %d\n",shmid);

		shmAddr = shmat(shmid,NULL,0);

		memset(shmAddr, 0,config->shSize);

		// pass this argument as parameter to another process, opening SHM.
		//cout << "------->> SHM Handle: " << handle << "------- " << endl;
		// TODO: improvement: write this to file, which can be read from another process. 
		// This can also help manage race condition of 2 processes trying to create SHM at the same time by opening file in exlcusive mode.
	}
	else {

		cout << "Opening Shared Memory" << endl;

		int shmid=shmget((key_t)2345, config->shSize, 0666);
		printf("Key of shared memory is %d\n",shmid);
		shmAddr	= shmat(shmid,NULL,0);
	}


	if (shmAddr == nullptr) {
		cout << "SHM Creation Failed." << endl;
		exit(EXIT_FAILURE);
	}

	cout << "\nConfig: \n create/open: " << config->shCreate << endl;
	cout << "Name: " << config->shName << endl;
	cout << "Size: " << config->shSize << endl;

	cout << "\nShared Memory address : " << (UInt32 *)shmAddr << endl;

	return shmAddr;
}

/*
BPtr MemoryMgr::InternalGetSharedMemory_Managed(DicConfig * config) {

	BPtr shmAddr;

	cout << "Config: \n create/open: " << config->shCreate << endl;
	cout << "Name: " << config->shName << endl;
	cout << "Size: " << config->shSize << endl;
	cout << "Handle: " << config->handle << endl;

	if (config->shCreate) {

		// remove old SHM with same name.
		shared_memory_object::remove(config->shName);

		cout << "Creating Shared Memory" << endl;

		segment = new managed_shared_memory(create_only, config->shName, config->shSize);
    	shmAddr = (BPtr)segment->allocate(config->shSize);

		managed_shared_memory::handle_t handle = segment->get_handle_from_address(shmAddr);

		// pass this argument as parameter to another process, opening SHM.
		cout << "------->> SHM Handle: " << handle << "------- " << endl;
		// TODO: improvement: write this to file, which can be read from another process. 
		// This can also help manage race condition of 2 processes trying to create SHM at the same time by opening file in exlcusive mode.
	}
	else {

		cout << "Opening Shared Memory" << endl;
		segment = new managed_shared_memory(open_only, config->shName);
		shmAddr = (BPtr)segment->get_address_from_handle(config->handle);
	}


	if (shmAddr == nullptr)
		exit(EXIT_FAILURE); // possibly due to race condition. 2 parallel processes trying to create 

	if (shmAddr == nullptr) {
		cout << "SHM Creation Failed." << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Shared Memory address : " << (UInt32)shmAddr << endl;

	return shmAddr;
}
*/


VPtr MemoryMgr::InternalGetSharedMemory(DicConfig * config) {

	VPtr shmAddr;

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

	shmAddr = (VPtr)region->get_address();

	if (shmAddr == nullptr) {
		cout << "SHM Creation Failed." << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Shared Memory address : " << (UInt64)shmAddr << endl;

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
	
UInt32 MemoryMgr::AllocMem(UInt32 size) {

	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	if ((metaData->freeOffset + size) >= metaData->shmSize)
		exit(EXIT_FAILURE); // TODO: handle graciously. Try to extend SHM, if not we could exit?

	VPtr ptr = (VPtr)((BPtr)shmPtr + metaData->freeOffset);
	UInt32 ret = metaData->freeOffset;

	metaData->freeOffset += size;

	cout << "MemMgr: shmPtr: " << (UInt32 *)shmPtr << ", ptr: " << (UInt32 *)ptr << "(" << ret 
		 << "). New offset: " << metaData->freeOffset << endl;

	return ret;
}

VPtr MemoryMgr::GetAppDataBuff(UInt32 offset) {


	scoped_lock<interprocess_mutex> lock(metaData->mutex);

	VPtr ret = (VPtr)((BPtr)shmPtr +  sizeof(MemMgrMetaData) + offset);

	cout << "MemMgr: shmPtr: " << (UInt32 *)shmPtr << ", AppData: " << (UInt32 *)ret << endl;

	return ret;
}

VPtr MemoryMgr::Ptr(UInt32 offset) {
	return (VPtr)((BPtr)shmPtr+offset);
}

void MemoryMgr::sleepfor(UInt64 msec) {

    std::chrono::milliseconds timespan(msec); // or whatever
    std::this_thread::sleep_for(timespan);
}