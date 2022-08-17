#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Posix SHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>

// added below namespaces just to avoid using their names everywhere.
using namespace std;

typedef unsigned long long  UInt64;
typedef long long           Int64 ;
typedef unsigned int       	UInt32;
typedef int                	Int32 ;
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