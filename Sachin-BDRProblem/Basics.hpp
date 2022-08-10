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

// TODO: we are initializing SHM with 0.25 GB in one shot. TODO: All SHM memory is allocated in beginning. 
// Improve: initialize SHM with less memory first & then expand SHM as and when required.
#define DEFAULT_SHM_SIZE					2^28 
#define SHM_NAME_SIZE						32

// TODO: this is to avoid fragmentation of memory in case of word alter or word delete & then create again.
// Improve: Use whatever memory required for word & handle defragmentation of memory whenever alter is called with extra space than earlier.

#define WORD_DEF_MAX_SZIE					200	 

struct DicConfig {

	bool			shCreate;	// true: create SHM, false: open SHM
	char			shName[SHM_NAME_SIZE];
	unsigned int	shSize; 
};

enum DicStatus {

	DIC_SUCCESS = 0,
	DIC_ERROR,

	// boost library errors related to interprocess mutex or shared memory.
	BOOST_LIB_ERR = 100,

	// Memory Errors.
	MEM_UNAVAILABLE = 200,
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