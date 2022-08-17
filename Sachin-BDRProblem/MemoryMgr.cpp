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

void mutex_init(pthread_mutex_t *mutex)
{

	// mutex init
	int rc;
	pthread_mutexattr_t attr;

	rc = pthread_mutexattr_init(&attr);
	if (!rc)
	{
		rc = pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
		if (!rc)
		{
			rc = pthread_mutex_init(mutex, &attr);
			if (!rc)
			{
				rc = pthread_mutexattr_destroy(&attr);
			}
		}
	}

	if (rc)
		exit(EXIT_FAILURE);

	// TODO: handle erros as exceptions
}

MemoryMgr::MemoryMgr(DicConfig *config) {


	shmPtr = InternalGetSharedMemory_Posix(config);

	Initialize(config);
	// TODO: handle race condition if 2 processes try to createSHM at the same time.
	// We can open file in exclusive access mode, write SHM key to it & use it in another process to open SHM.
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

	// hardcoded key below. TODO: Change those to macro/constants. Also we can pass that to other processes thorough disk file.
	if (config->shCreate) {

		cout << "Creating Shared Memory" << endl;

		int shmid =shmget((key_t)2345, config->shSize, 0666|IPC_CREAT); 
		
		printf("Key of shared memory is %d\n",shmid);

		shmAddr = shmat(shmid,NULL,0);

		memset(shmAddr, 0,config->shSize);

		// TODO: handle race condition if 2 processes try to createSHM at the same time.
		// We can open file in exclusive access mode, write key to it & use it in another process to open SHM.
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


void MemoryMgr::Initialize(DicConfig * config) {

	if (config->shCreate) {

		// allocate MemMgrMetaData
		metaData = new (shmPtr) MemMgrMetaData;

		mutex_init(&metaData->mutex);

		if (pthread_mutex_lock(&metaData->mutex)) {
			
			cout << "SHM Errory. Exiting.\n";
            exit(EXIT_FAILURE);
		}

		metaData->shmSize		= config->shSize;
		memcpy(metaData->shName, config->shName, SHM_NAME_SIZE);

		metaData->freeOffset	= sizeof(MemMgrMetaData);

		cout << "MemMgr current offset: " << metaData->freeOffset << endl;

		if (pthread_mutex_unlock(&metaData->mutex)) {
			cout << "SHM Errory. Exiting.\n";
			exit(EXIT_FAILURE);
		}
	}
	else {

		// allocate MemMgrMetaData
		metaData =  (MemMgrMetaData *)shmPtr;

		if (pthread_mutex_lock(&metaData->mutex)) {
			
			cout << "SHM Errory. Exiting.\n";
            exit(EXIT_FAILURE);
		}

		if (strncmp(metaData->shName, config->shName, SHM_NAME_SIZE) != 0) {

			cout << "Something wrong in SHM\n";
			exit(EXIT_FAILURE);
		}
		if (pthread_mutex_unlock(&metaData->mutex)) {
			
			cout << "SHM Errory. Exiting.\n";
            exit(EXIT_FAILURE);
		}
	}

}

DicStatus	MemoryMgr::DeInitialize() {

	DicStatus rc = DIC_SUCCESS;
	
	// TODO: what is someone is using SHM currently? Handle graceful shutdown.

	MemoryMgr * obj = MemoryMgr::Obj();

	if (pthread_mutex_lock(&obj->metaData->mutex)) {
		
		cout << "SHM Errory. Exiting.\n";
		exit(EXIT_FAILURE);
	}

	// TODO: add SHM remove function. Code is not added currently because other process may be using it. This needs to be handled gracefully.

	instance = nullptr;

	if (pthread_mutex_unlock(&obj->metaData->mutex)) {
			
			cout << "SHM Errory. Exiting.\n";
            exit(EXIT_FAILURE);
		}
	return rc;
}	
	
UInt32 MemoryMgr::AllocMem(UInt32 size) {


	if (pthread_mutex_lock(&metaData->mutex)) {
			
		cout << "SHM Errory. Exiting.\n";
		exit(EXIT_FAILURE);
	}

	if ((metaData->freeOffset + size) >= metaData->shmSize)
		exit(EXIT_FAILURE); // TODO: handle graciously. Try to extend SHM, if not we could exit?

	UInt32 ret = metaData->freeOffset;

	metaData->freeOffset += size;

	if (pthread_mutex_unlock(&metaData->mutex)) {
			
		cout << "SHM Errory. Exiting.\n";
		exit(EXIT_FAILURE);
	}

	return ret;
}

VPtr MemoryMgr::GetAppDataBuff(UInt32 offset) {

	VPtr ret = (VPtr)((BPtr)shmPtr +  sizeof(MemMgrMetaData) + offset);

	return ret;
}

VPtr MemoryMgr::Ptr(UInt32 offset) {
	return (VPtr)((BPtr)shmPtr+offset);
}
