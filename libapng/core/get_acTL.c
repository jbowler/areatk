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

bool APNGAPI
apng_get_acTL(png_const_structp png_ptr, png_infop info_ptr,
   png_uint_32p num_frames_ptr, png_uint_32p num_plays_ptr)
{
   static const png_byte acTL[] = APNG_acTL_str;
   static const png_byte fcTL[] = APNG_fcTL_str;
   static const png_byte fdAT[] = APNG_fdAT_str;

   if (png_handle_as_unknown(png_ptr, acTL) < PNG_HANDLE_CHUNK_IF_SAFE ||
       png_handle_as_unknown(png_ptr, fcTL) < PNG_HANDLE_CHUNK_IF_SAFE ||
       png_handle_as_unknown(png_ptr, fdAT) < PNG_HANDLE_CHUNK_IF_SAFE)
      png_error(png_ptr, "apng_get_acTL: missing APNG handling");

   png_unknown_chunkp chunks = NULL;
   const int num = png_get_unknown_chunks(png_ptr, info_ptr, &chunks);
   const int next = apng_search(num, chunks, 0, APNG_FIND_acTL);

   if (next < num)
   {
      if (chunks[next].size == 8U &&
          (chunks[next].location & PNG_AFTER_IDAT) == 0U)
      {
         const png_uint_32 num_frames = APNG_U32_RGB(chunks[next].data + 0U);
         const png_uint_32 num_plays = APNG_U32_RGB(chunks[next].data + 4U);

         /* Check for PNG unsigned four byte values: */
         if (((num_frames | num_plays) & 0x80000000U) == 0U &&
             num_frames > 0U)
         {
            if (num_frames_ptr != NULL)
               *num_frames_ptr = num_frames;

            if (num_plays_ptr != NULL)
               *num_plays_ptr = num_plays;

            return true;
         }
      }
   }

   return false;
}
