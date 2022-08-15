#pragma once

#include <iostream>
#include <string>

// Boost library shared memory.
#include <boost/interprocess/managed_shared_memory.hpp>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

// Boost library inter-process mutexes.
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <boost/core/no_exceptions_support.hpp>

// TODO: debugging purpose. remove later
#include <chrono>
#include <thread>

// Posix SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

// added below namespaces just to avoid using their names everywhere.
using namespace std;
using namespace boost::interprocess;

typedef unsigned long long  UInt64;
typedef long long           Int64 ;
typedef unsigned long       UInt32;
typedef long                Int32 ;
typedef unsigned short      UInt16;
typedef short               Int16 ;
typedef unsigned char       UInt8;
typedef char                Int8 ;
typedef unsigned char *     BPtr;
typedef void *				VPtr;

// TODO: we are initializing SHM with 128 MB in one shot. TODO: All SHM memory is allocated in beginning. 
// Improve: initialize SHM with less memory first & then expand SHM as and when required.
#define SHM_NAME_SIZE						32

struct DicConfig {

	bool			shCreate;	// true: create SHM, false: open SHM
	Int8			shName[SHM_NAME_SIZE];
	UInt32			shSize; 

	UInt64 			handle; // TODO: debugging
};

enum DicStatus {

	DIC_SUCCESS = 0,

	// Dict layer errors
	DIC_INVALID_IP_PARAM,

	// Trie Store Errors
	TST_WORD_ALREADY_EXISTS = 100,
	TST_WORD_DOESNOT_EXIST,

	// Memory Errors.
	MEM_UNAVAILABLE = 200,
	MEM_INIT_ERROR,

	// boost library errors related to interprocess mutex or shared memory.
	BOOST_LIB_ERR = 300,

	// pthread errors
	PTHREAD_LOCK_ERROR,

};