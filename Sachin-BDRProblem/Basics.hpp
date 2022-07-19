#pragma once

#include <iostream>
#include <string>

using namespace std;

typedef unsigned long long  UInt64;
typedef long long           Int64 ;
typedef unsigned long       UInt32;
typedef long                Int32 ;
typedef unsigned short      UInt16;
typedef short               Int16 ;
typedef unsigned char       UInt8;
typedef char                Int8 ;
typedef unsigned char *     BPtr;

enum DicStatus {

	DIC_SUCCESS = 0,
	DIC_ERROR,

	// Sync/Atomics errors.
	SYNC_OTHERS_HAVE_EXCL_ACCESS = 100,
	SYNC_OTHERS_HAVE_SHARED_ACCESS,

	// Memory Errors.
	MEM_UNAVAILABLE = 200,
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