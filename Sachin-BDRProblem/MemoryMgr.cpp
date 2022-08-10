#pragma once

#include "Basics.hpp"


struct BucketNode {

	BPtr *		memChunk;
	UInt32		chunkSize;
	BucketNode* nextChunk; // list of all the free chunks in a given bucket.
};

struct MemMgrMetaData {

	interprocess_mutex 	mutex;						// from boost library.
	char				shName[SHM_NAME_SIZE];
	UInt32				appDataSize;
	UInt32				freeOffset;					// TODO: Temperary. Currently memory is always assinged from this offset. 
														// See AllocMem function for actual design
};

/* ---------- MemoryMgr -----------
* 1) Manage Shared Memory, new allocations or deletions, etc.
* 2) Use Buckets to manage fragmeted memory starting from 32 bytes till 64k size buckets? 
*	a) Minimum 32 bytes of memory chunks are tracked.
	b) Total 16 (2^16 = 64k) buckets are defined but first 4 are not used for easy calculation. 
*	c) bucket of size 'X' tracks all memory chunks with atleast 'X' bytes till next bucket size.
*	d) 
* 3) When to defragment memory?
*	a) Every time DeAllocData() is called?
*	b) In background thread?
* 4) All functions & variables are static.
*/

class MemoryMgr {

public:

	/* ------------ Initialize() ------------
	* 1) This is called when process starts. If SHM is already allocated, reads(if valid data) metadata of the same.
	* 2) If not, it creates file in exclusive mode(so no other process can read partial file or uninitialized SHM), 
		creates SHM, writes appdata to SHM & finally writes metadata to given file & closes file. 
	* 3) There could be various race conditions:
	*	a) File is corrupt (earlier process couldn't finish successful creation of SHM)
	*   b) 2 processes trying to create SHM in concurrently. Exlusive mode of file should prevent this.	*   
	*   c) Other process is trying to read metadata as file is already created but this process hasn't finished writing initializing SHM yet.
	*		Again Excl mode should prevent this.
	*	d) File has valid content, but SHM got destroyed because OS restarted: outside the scope of this assigment.
	*	e) Other process trying to destroy already created SHM (because user requested in end).	*		
	*/

	static DicStatus InternalGetSharedMemory(DicConfig config) {

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
			return MEM_BOOST_ERR;

		} BOOST_CATCH_END

		return DIC_SUCCESS;
	}

	static DicStatus Initialize(DicConfig & config, const BPtr appdata, UInt32 appDataSize) {

		// ip validations?

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

				metaData->appDataSize = appDataSize;
				metaData->freeOffset = sizeof(MemMgrMetaData);

				// write app data.
				memcpy(shmPtr + metaData->freeOffset, appdata, appDataSize);
				metaData->freeOffset += appDataSize;

				// Last SHM name in last. This is to handle race condition with Open SHM: if in case this process crashes before this step.
				memcpy(metaData->shName, config.shName, SHM_NAME_SIZE);
			}
			else {

				// allocate MemMgrMetaData
				metaData =  (MemMgrMetaData *)shmPtr;

				// allocate MemMgrMetaData
				scoped_lock<interprocess_mutex> lock(metaData->mutex);
				cout << "MemoryMgr: Mutex locked for OpenMgr." << endl;

				if (strncmp(metaData->shName, config.shName, SHM_NAME_SIZE) != 0)
					rc = MEM_INIT_ERROR;
			}

			cout << "MemoryMgr Mutex UnLocked." << endl;
		}

		return rc;
	}
	static DicStatus	DeInitialize(bool Create, const string shareMemName, const BPtr appdata, UInt32 appDataSize) {

		BOOST_TRY {

			// TODO: Handle more scenarios of existing gracefully.
			scoped_lock<interprocess_mutex> lock(metaData->mutex);

			shared_memory_object::remove(config.shName);
		}
		BOOST_CATCH(interprocess_exception& ex) {

			cout << "Execption: " << ex.what() << endl;
			return MEM_BOOST_ERR;

		} BOOST_CATCH_END
		
		return DIC_SUCCESS;
	}

	/*--------- All below functions should be called only after Initialize ---------*/
	// reads data from SHM with given size starting from 'MEM_APPDATA_START_OFFSET + startoffset'. 
	
	static DicStatus	GetAppData(BPtr & appData, UInt32 & appDataSize) {

		appDataSize = metaData->appDataSize;

		memcpy(appData, shmPtr + sizeof(MemMgrMetaData), appDataSize);
		return DIC_SUCCESS;
	}

	/* --------------- AllocData / DeAllocData ----------------
	* 1) For AllocData: 
	*	a) First take exl sync on, we calculate 'power of 2' bucket size higher than requested size, 
	*		goto respective bucket & issue memory from first memory chunk, adjust remaining chunk in same or lower bucket.
	*   b) Reset the memory before returning.
	*	c) If SHM memory is exhausted, more SHM memory can be added & stored in various buckets.
	*		i) Will need to extend shmPtr to probably to Dynamic Array or LinkedList.
	* 2) Similarly for DeAllocData...
	*/
	static BPtr AllocMem(UInt32 size) {

		/* TODO: This is not implemented completely because of lack of time.
		* Here is the Design:
		* All freed memory chunks (continuous free memory) are stored in various buckets sizes.
		* Buckets are of size: 32, 64, 128, 256, 512, 1024, .... upto 2^16. as not expecting any freed memory to be above 2^16, which itself is an overkill.
		* e.g. : 1) chunk of sizes from 33 till 64 are stored in a bucket size of 64. Similarly chunks of sizes from 65 till 128 are stored in bucket of size 128.
		* While assigning memory would be assigned from free chunks from a bucket size of upperbound of requested memory size.
		*
		*/

		// TODO: Temperary logic: Always assign memory from monotonically growing offset. Ieally above logic of managed free chunks (in various size buckets) would be used.
		
		scoped_lock<interprocess_mutex> lock(metaData->mutex);

		BPtr outptr = shmPtr + metaData->freeOffset;
		metaData->freeOffset += size;

		return outptr;
	}
	static DicStatus DeAllocMem(const BPtr /*ptr*/, UInt32 /*size*/) {

		/* TODO: This is not implemented because of lack of time.
		* Here is the Design:
		* All freed memory chunks (continuous free memory) are stored in various buckets sizes.
		* Buckets are of size: 32, 64, 128, 256, 512, 1024, .... upto 2^16. as not expecting any freed memory to be above 2^16, which itself is an overkill.
		* e.g. : 1) chunk of sizes from 33 till 64 are stored in a bucket size of 64. Similarly chunks of sizes from 65 till 128 are stored in bucket of size 128.
		* While assigning memory would be assigned from free chunks from a bucket size of upperbound of requested memory size.
		* 
		*/

		scoped_lock<interprocess_mutex> lock(metaData->mutex);
		// TODO

		return DIC_SUCCESS;
	}

private:

	// Defined private in order to prevent instantiating this class as all members are used as static.
	MemoryMgr() {};
	
	static MemMgrMetaData *		metaData;	
	static BPtr					shmPtr;		
};