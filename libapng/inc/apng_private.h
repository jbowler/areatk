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

/* ASCII strings for the relevant chunk names: */
/* IDAT */
#define APNG_IDAT_str "\x49\x44\x41\x54"

#define APNG_CHUNKS_IDAT APNG_IDAT_str

/* APNG CHUNKS: */
#define APNG_acTL_str "\x61\x63\x54\x4C"
#define APNG_fcTL_str "\x66\x63\x54\x4C"
#define APNG_fdAT_str "\x66\x64\x41\x54"

#define APNG_CHUNKS_APNG APNG_acTL_str "\0" APNG_fcTL_str "\0" APNG_fdAT_str

/* COLOUR-SPACE CHUNKS: */
#define APNG_cHRM_str "\x63\x48\x52\x4D"
#define APNG_gAMA_str "\x67\x41\x4D\x41"
#define APNG_iCCP_str "\x69\x43\x43\x50"
#define APNG_sBIT_str "\x73\x42\x49\x54"
#define APNG_sRGB_str "\x73\x52\x47\x42"

#define APNG_CHUNKS_COL APNG_cHRM_str "\0" APNG_gAMA_str "\0"\
   APNG_iCCP_str "\0" APNG_sBIT_str "\0" APNG_sRGB_str

/* New with PNGv3, not supported in older libpng builds: */
#define APNG_cICP_str "\x63\x49\x43\x50"
#define APNG_mDCV_str "\x6D\x44\x43\x56"
#define APNG_cLLI_str "\x63\x4C\x4C\x49"

#define APNG_CHUNKS_COLv3 APNG_cICP_str "\0" APNG_mDCV_str "\0" APNG_cLLI_str

/* TEXT CHUNKS: */
#define APNG_tEXt_str "\x74\x45\x58\x74"
#define APNG_zTXt_str "\x7A\x54\x58\x74"
#define APNG_iTXt_str "\x69\x54\x58\x74"

#define APNG_CHUNKS_TEXT APNG_tEXt_str "\0" APNG_zTXt_str "\0" APNG_iTXt_str

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

/* INTERNAL FUNCTIONS */
APNGPRIVATE int apng_search(int num, png_unknown_chunkp chunks, int next,
   unsigned what);
   /* Given a chunk list return the index of the next chunk whose name matches
    * one of the APNG chunks given by 'what', a bitwise mask of PNG_FIND_
    * values.  The search starts at chunks[next] and returns -1 if no match is
    * found.
    */

#endif /* !APNG_PRIVATE_H */
