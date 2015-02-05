#pragma once

#include <stdint.h>

#ifdef __MINGW32__
#include <excpt.h>
#ifndef TRYLEVEL_NONE
#ifndef __MINGW64__
#define NO_SEH_MINGW
#endif
#ifndef __try
#define __try
#endif
#ifndef __except
#define __except(x) if (0)
#endif
#endif
#endif

static inline int safe_memcmp(const void *p1, const void *p2, size_t size)
{

#ifdef NO_SEH_MINGW
	__try1(EXCEPTION_EXECUTE_HANDLER)
#else
	__try
#endif
	{
		return memcmp(p1, p2, size);
	}
#ifdef NO_SEH_MINGW
	__except1
#else
	__except(EXCEPTION_EXECUTE_HANDLER)
#endif
	{
		return -1;
	}

}

struct patch_info {
	size_t size;
	const BYTE *data;
};

#define NEW_PATCH(x) {sizeof(x), (x)}
#define MAX_PATCH_SIZE 2

static const BYTE force_jump[] = {0xEB};
static const BYTE ignore_jump[] = {0x90, 0x90};

#ifdef _WIN64

#define NUM_VERS   (10)
#define CMP_SIZE   (13)

static const uintptr_t patch_offset[NUM_VERS] = {
	0x54FE6,  //win7   - 6.1.7600.16385
	0x55095,  //win7   - 6.1.7601.16562
	0x550C5,  //win7   - 6.1.7601.17514
	0x8BDB5,  //win8.1 - 6.3.9431.00000
	0x8E635,  //win8.1 - 6.3.9600.17415
	0x90352,  //win8.1 - 6.3.9600.17085
	0x9038A,  //win8.1 - 6.3.9600.17095
	0x93AFA,  //win8.1 - 6.3.9600.16384
	0x93B8A,  //win8.1 - 6.3.9600.16404
	0x1841E5  //win8   - 6.2.9200.16384
};

static const uint8_t patch_cmp[NUM_VERS][CMP_SIZE] = {
{0x48, 0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x39, 0x98, 0x68, 0x50, 0x00, 0x00},
{0x48, 0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x39, 0x98, 0x68, 0x50, 0x00, 0x00},
{0x48, 0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x39, 0x98, 0x68, 0x50, 0x00, 0x00},
{0x48, 0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x39, 0xB0, 0x28, 0x51, 0x00, 0x00},
{0x48, 0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x39, 0xA8, 0x28, 0x51, 0x00, 0x00},
{0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x44, 0x39, 0xA0, 0x28, 0x51, 0x00, 0x00},
{0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x44, 0x39, 0xA0, 0x28, 0x51, 0x00, 0x00},
{0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x44, 0x39, 0xA0, 0x28, 0x51, 0x00, 0x00},
{0x8b, 0x81, 0xb8, 0x3d, 0x00, 0x00, 0x44, 0x39, 0xA0, 0x28, 0x51, 0x00, 0x00},
{0x49, 0x8b, 0x85, 0xb8, 0x3d, 0x00, 0x00, 0x39, 0x88, 0xc8, 0x50, 0x00, 0x00},
};

static const struct patch_info patch[NUM_VERS] = {
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(ignore_jump),
	NEW_PATCH(ignore_jump),
	NEW_PATCH(ignore_jump),
	NEW_PATCH(ignore_jump),
	NEW_PATCH(ignore_jump),
	NEW_PATCH(ignore_jump),
	NEW_PATCH(ignore_jump),
};

#else

#define NUM_VERS   (10)
#define CMP_SIZE   (12)

static const uintptr_t patch_offset[NUM_VERS] = {
	0x79AA6,  //win7   - 6.1.7601.16562
	0x79C9E,  //win7   - 6.1.7600.16385
	0x79D96,  //win7   - 6.1.7601.17514
	0x7F9BD,  //win8.1 - 6.3.9431.00000
	0x8A3F4,  //win8.1 - 6.3.9600.16404
	0x8E9F7,  //win8.1 - 6.3.9600.17095
	0x8F00F,  //win8.1 - 6.3.9600.17085
	0x8FBB1,  //win8.1 - 6.3.9600.16384
	0x90264,  //win8.1 - 6.3.9600.17415
	0x166A08  //win8   - 6.2.9200.16384
};

static const uint8_t patch_cmp[NUM_VERS][CMP_SIZE] = {
{0x8b, 0x89, 0xe8, 0x29, 0x00, 0x00, 0x39, 0xb9, 0x80, 0x4b, 0x00, 0x00},
{0x8b, 0x89, 0xe8, 0x29, 0x00, 0x00, 0x39, 0xb9, 0x80, 0x4b, 0x00, 0x00},
{0x8b, 0x89, 0xe8, 0x29, 0x00, 0x00, 0x39, 0xb9, 0x80, 0x4b, 0x00, 0x00},
{0x8b, 0x80, 0xe8, 0x29, 0x00, 0x00, 0x39, 0xb0, 0x40, 0x4c, 0x00, 0x00},
{0x80, 0xe8, 0x29, 0x00, 0x00, 0x83, 0xb8, 0x40, 0x4c, 0x00, 0x00, 0x00},
{0x80, 0xe8, 0x29, 0x00, 0x00, 0x83, 0xb8, 0x40, 0x4c, 0x00, 0x00, 0x00},
{0x80, 0xe8, 0x29, 0x00, 0x00, 0x83, 0xb8, 0x40, 0x4c, 0x00, 0x00, 0x00},
{0x80, 0xe8, 0x29, 0x00, 0x00, 0x83, 0xb8, 0x40, 0x4c, 0x00, 0x00, 0x00},
{0x87, 0xe8, 0x29, 0x00, 0x00, 0x83, 0xb8, 0x40, 0x4c, 0x00, 0x00, 0x00},
{0x8b, 0x80, 0xe8, 0x29, 0x00, 0x00, 0x39, 0x90, 0xb0, 0x4b, 0x00, 0x00},
};

static const struct patch_info patch[NUM_VERS] = {
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(force_jump),
	NEW_PATCH(ignore_jump),
	NEW_PATCH(force_jump),
};

#endif

static inline int get_d3d9_patch(HMODULE d3d9)
{
	uint8_t *addr = (uint8_t*)d3d9;
	for (int i = 0; i < NUM_VERS; i++) {
		int ret = safe_memcmp(addr + patch_offset[i], patch_cmp[i],
				CMP_SIZE);
		if (ret == 0)
			return i;
	}

	return -1;
}

static inline uint8_t *get_d3d9_patch_addr(HMODULE d3d9, int patch)
{
	if (patch == -1)
		return nullptr;

	uint8_t *addr = (uint8_t*)d3d9;
	return addr + patch_offset[patch] + CMP_SIZE;
}
