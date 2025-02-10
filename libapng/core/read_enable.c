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
#  include "../inc/apng_private.h"
#endif

#define APNG_CHUNK_LIST APNG_acTL_str "\0" APNG_fcTL_str "\0" APNG_fdAT_str

/* png_set_keep_unknown_chunks takes a png_const_bytep, so: */
static const png_byte chunk_list[] = APNG_CHUNK_LIST;

bool APNGAPI
apng_read_enable(png_structrp png_ptr) {
   png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS,
         chunk_list, (sizeof chunk_list));
   return true;
}
