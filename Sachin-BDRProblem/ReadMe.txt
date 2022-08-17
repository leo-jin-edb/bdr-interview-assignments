------------------------------------------------------
Usage:

// Here is how program needs to be started by the first process trying to create SHM 
./Dictionary create 

// Here is how program needs to be started by any process after creating Shared memory.
./Dictionary open

------------------------------------------------------

Assumptions:
1) System would be read (Search) heavy. Probably 80-20% split. 20% could be Insert or Delete.
	a) Because in code, we are taking exclusive access at trie node level whether we are doing read/write at that node level. 

------------------------------------------------------
System Further Improvements:
1) MemoryMgr layer do not handle memory defragmentation as of now & hence at every delete, memory is leaked. This can be handled by maintaining various Buckets of list of freed memory segments of various sizes like 32/64/128/256/512/1024/ etc bytes. Currenly memory is alwasy allocated from tail end of used memory so far.

2) System shutdown doesn't cover graceful cleanup. Race conditions like trying to close the SHM while others are using SHM. Hence Destructors are not added yet in order for program to shutdown without removing SHM.

3) While creating SHM, key has been hardcoded currently. Also race condition if 2 processes trying to create SHM at the same time has not been handled. 
THis can be improved further by writing key to a disk file which can be opened in exclusive mode by a process trying to create SHM & shared mode by process just opening the SHM.

4) SHM name is restricted to 32 bytes, can be changed to have a variable size name.

5) Extending SHM size if it gets used completely, has not been handled. We are simply exiting an application if there is no memory. We also exit application currently if there is any error during memory allocation or pthread artifacts of SHM or mutex, etc.



