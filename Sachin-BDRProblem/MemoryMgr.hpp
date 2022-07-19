#pragma once

#include "SyncAtomics.hpp"

#define MEM_SHM_METADATA_SIZE				64 // 64 bytes. TODO: adjust size as per real requirement. Any reserve space to be left?
#define MEM_APPDATA_START_OFFSET			64
#define	MEM_NUM_BUCKETS						16 
#define MEM_MINIMUM_BUCKET_SIZE				32
#define MEM_MAX_BUCKET_SIZE					65536

struct BucketNode {

	BPtr *		memChunk;
	UInt32		chunkSize;
	BucketNode* nextChunk;
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

	static DicStatus	Initialize(const string /*shareMemFilePath*/, const BPtr /*appdata*/, UInt32 /*appDataSize*/) {
		return DIC_SUCCESS;
	}

	/*--------- All below functions should be called only after Initialize ---------*/
	// reads data from SHM with given size starting from 'MEM_APPDATA_START_OFFSET + startoffset'. 
	
	static DicStatus	GetAppData(BPtr& /*data*/, UInt32 /*appDataSize*/) {
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
	static DicStatus	AllocData(BPtr& /*ptr*/, UInt32 /*size*/) {
		return DIC_SUCCESS;
	}
	static DicStatus	DeAllocData(const BPtr /*ptr*/, UInt32 /*size*/) {
		return DIC_SUCCESS;
	}

private:

	// Defined private in order to prevent instantiating this class as all members (functions + variables) are static.
	MemoryMgr() {};

	static UInt16		appDataSize;
	static SyncFlag		memSync; // 
	static BPtr			shmPtr;	// shared memory ptr.
	static BucketNode	buckets[MEM_NUM_BUCKETS];
};