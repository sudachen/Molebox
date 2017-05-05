
/*

  Copyright (C) 2014, Alexey Sudachen, https://goo.gl/lJNXya.

*/

#pragma once
#include <stdint.h>

typedef void SVFS_PACKAGE;
typedef void SVFS_FIPROXY;

enum { SVFS_FILE_BLOCK_SIZE = 48 * 1024 };

enum
{
    SVFS_HIDDEN     = 1L,
    SVFS_COMPRESSED = 1L << 1,
    SVFS_ENCRYPTED  = 1L << 2,
    SVFS_ACTIVEX    = 1L << 3,
    SVFS_EXECUTABLE = 1L << 4,
    SVFS_EXTERNAL   = 1L << 5,
    SVFS_NOACCESS   = 1L << 6,
    SVFS_DIRECTORY  = 0x80000000,
};

struct SVFS_FILE_KEY
{
	uint32_t  crc;
	uint8_t   sign[16];
};

struct SVFS_PUBLIC_NAME
{
	const wchar_t* S;
	uint32_t       length;
};

struct SVFS_FILE_BLOCK
{
	uint16_t packed_size;
	uint16_t real_size;
	uint16_t flags;
	uint16_t crc;
	uint32_t offset;
};

struct SVFS_FILE_INFO
{
	SVFS_PACKAGE*     pkg;
	SVFS_FIPROXY*     proxy;
	SVFS_PUBLIC_NAME  name;
	SVFS_FILE_KEY     key;
	SVFS_FILE_INFO*   dir;

	uint32_t flags, size, count;

#ifndef SVFS_PACKER
	union
	{
		SVFS_FILE_BLOCK block[1];
		SVFS_FILE_INFO*    ls[1];
	};
#endif

};

struct SVFS_HEADER
{
	uint32_t tree_boffs;
	uint32_t tree_size;
	uint32_t root_roffs;
	uint32_t base_boffs;
	uint32_t crc;
	uint32_t map_roffs;
	uint32_t map_count;
	uint32_t reserved;
	uint32_t stream_ptr;
	uint32_t stream_size;
	uint8_t  signature[8];
};
