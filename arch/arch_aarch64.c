/*
 * MIT License
 *
 * Copyright (c) 2022 Davidson Francis <davidsondfgl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdint.h>
#include <string.h>

#include "arch.h"
#include "log.h"
#include "util.h"

#ifndef __aarch64__
#error "This file should be built for aarch64-only targets!"
#endif

/**/
static uintptr_t arch_addr_start;

/*
 *
 * @note: Instructions big-endian encoded.
 * x0/w0 should be preserved.
 */
static uint8_t patch[] = {
	/* ldr x1, 8 */
	0x41, 0x00, 0x00, 0x58,
	/* blr x1 */
	0x20, 0x00, 0x3f, 0xd6,
	/* target address to be loaded */
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

/**/
static uint8_t bck_start[sizeof patch] = {0};

/**
 *
 *
 * @return Returns the amount needed to be decremented
 * in order to fix the return address.
 */
size_t arch_restore_start(void) {
	memcpy((char*)arch_addr_start, bck_start, sizeof bck_start);
	return (sizeof bck_start) - 8; /* desconsider target addr size. */
}

/**
 *
 */
int arch_patch_start(uintptr_t start)
{
	COMPILE_TIME_ASSERT(sizeof(uintptr_t) == sizeof(uint64_t));

	int i;

	union
	{
		int64_t addr;
		uint8_t val[8];
	} u_addr;

	arch_addr_start = start;

	/* Backup the bytes that we're overwritten. */
	memcpy(bck_start, (void*)start, sizeof patch);

	/* Calculate rel-offset. */
	u_addr.addr = (uint64_t)arch_pre_daemon_main;

	/* Patch: constant to be loaded. */
	for (i = 0; i < 8; i++)
		patch[8 + i] = u_addr.val[i];

	memcpy((void*)start, patch, sizeof patch);
	return (0);
}
