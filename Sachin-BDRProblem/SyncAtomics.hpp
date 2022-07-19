
#pragma once
/*
	Enables thread sync functionalities, etc.
*/
#include "Basics.hpp"


/*
 * 1) Implements mutex/semaphore kind of functionality using CMP & SWAP command.
 * 2) Manages Exclusive/Shared access to requested Sync-Flag
 */
class SyncFlag {

public:
	SyncFlag() {
		sFlag = 0;
	}

	// For Exlcusive access updates value of requested Sync-Flag to 0x80000000 using CmpSwap instruction only if current value is 0.
	bool AccessExclusive() { 
		// TODO: complete
		return DIC_SUCCESS;
	};

	// For Shared access increaments the value of requested Sync - Flag by 1 only current value is not '0x80000000'.
	bool AccessShared() {
		// TODO: complete
		// check for max read concurrency

		return DIC_SUCCESS;
	}; 

	void ReleaseExclusive() {
		// TODO: complete
	};

	void ReleaseShared() {
		// TODO: complete
	};

private:

	// Assumption: max concurrency we want is in few dozens (under 128, MSB bit used for Exclusive access). Hence taken 1 byte variable.
	volatile UInt8 sFlag;
};

/*
* 1) Reads value of requested variable atomically.
*/
class Atomics {



};