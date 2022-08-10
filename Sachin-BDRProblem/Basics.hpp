#pragma once

#include <iostream>
#include <string>

// Boost library shared memory.

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>

// Boost library inter-process mutexes.
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

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

#define	MEM_NUM_BUCKETS						16 
#define MEM_MINIMUM_BUCKET_SIZE				32
#define MEM_MAX_BUCKET_SIZE					65536 // overkill for now. We will hardly need such a high bucket size for storing free memory chunks.

#define SHM_NAME_SIZE						32
#define WORD_DEF_MAX_SZIE					256	 // this is to avoid fragmentation of memory in case of word alter or word delete & then create again.

struct DicConfig {

	bool			shCreate;	// true: create SHM, false: open SHM
	char			shName[SHM_NAME_SIZE];
	unsigned int	shSize; 
};

enum DicStatus {

	DIC_SUCCESS = 0,
	DIC_ERROR,

	// Sync/Atomics errors.
	SYNC_OTHERS_HAVE_EXCL_ACCESS = 100,
	SYNC_OTHERS_HAVE_SHARED_ACCESS,

	// Memory Errors.
	MEM_UNAVAILABLE = 200,
	MEM_BOOST_ERR,
	MEM_INIT_ERROR,
	MEM_UNKNOWN_ERR,

	// Trie Store Errors
	TST_WORD_ALREADY_EXISTS = 300,
	TST_WORD_DOESNOT_EXIST,
	TST_MEMORY_FULL,
	TST_UNKNOWN_ERR,

	// Dic App Errors
	DIC_INVALID_IP_PARAM = 400,
	DIC_UNKNOWN_ERR,
};