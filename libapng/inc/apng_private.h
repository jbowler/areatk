/* This file is part of the APNG reading, editing and authoring toolkit.

Copyright (c) 2025 John Bowler

AREATK is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later
version.

AREATK is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
AREATK. If not, see <https://www.gnu.org/licenses/>. */

#ifndef APNG_PRIVATE_H
#define PNG_APNG_PRIVATE_H

#include <string.h>
#include "apng.h"

#ifndef APNGPRIVATE
#  define APNGPRIVATE
#endif

/* INTERNAL DEFINITIONS
 *
 * These are used only at build time.
 */
#define APNG_acTL APNG_CHUNKNAME(0x61, 0x63, 0x54, 0x4C)
#define APNG_fcTL APNG_CHUNKNAME(0x66, 0x63, 0x54, 0x4C)
#define APNG_fdAT APNG_CHUNKNAME(0x66, 0x64, 0x41, 0x54)

#define APNG_acTL_str "\x61\x63\x54\x4C"
#define APNG_fcTL_str "\x66\x63\x54\x4C"
#define APNG_fdAT_str "\x66\x64\x41\x54"

#define APNG_INVALID_SEQUENCE_NUMBER 0x80000000U

/* INTERNAL MACROS
 */
#define APNG_UADD(u, b) (((u) << 8) + (b))
#define APNG_U16(b0, b1)\
   APNG_UADD((0x7FFFU & (b0)), b1)
#define APNG_U32(b0, b1, b2, b3)\
   APNG_UADD(APNG_UADD(APNG_UADD((0xFFFFFFFFU & (b0)), b1), b2), b3)
#define APNG_CHUNKNAME(b0, b1, b2, b3) APNG_U32(b0, b1, b2, b3)

#define APNG_U16_RGB(rgb) APNG_U16((rgb)[0], (rgb)[1])
#define APNG_U32_RGB(rgb) APNG_U32((rgb)[0], (rgb)[1], (rgb)[2], (rgb)[3])

#define APNG_STORE_U8(rgb, index, u, shift)\
   ((rgb)[(index)] = ((u) >> (shift)) & 0xFFU)

#define APNG_STORE_U16(rgb, index, u16) ((void)(\
   APNG_STORE_U8(rgb, (index)+0U, u16, 8),\
   APNG_STORE_U8(rgb, (index)+1U, u16, 0)))

#define APNG_STORE_U32(rgb, index, u32) ((void)(\
   APNG_STORE_U8(rgb, (index)+0U, u32, 24),\
   APNG_STORE_U8(rgb, (index)+1U, u32, 16),\
   APNG_STORE_U8(rgb, (index)+2U, u32,  8),\
   APNG_STORE_U8(rgb, (index)+3U, u32,  0)))

/* APNG_CRC32 calculates a running crc without undoing the pre-conditioning.
 * This means it must be started with 0xFFFFFFFFU and the final value must be
 * XORed with that.
 */
#define APNG_CRC32(crc, rgb, len) apng_crc32((crc), (rgb), (len))

/* INTERNAL FUNCTIONS */
APNGPRIVATE int apng_search(int num, png_unknown_chunkp chunks, int next,
   unsigned what);
   /* Given a chunk list return the index of the next chunk whose name matches
    * one of the APNG chunks given by 'what', a bitwise mask of PNG_FIND_
    * values.  The search starts at chunks[next] and returns -1 if no match is
    * found.
    */

#endif /* !APNG_PRIVATE_H */
