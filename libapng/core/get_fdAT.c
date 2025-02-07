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
apng_get_fdAT(png_const_structp png_ptr, png_infop info_ptr, png_uint_32 sqn,
   png_uint_32p data_len_ptr, png_const_bytep *data_ptr)
{
   png_unknown_chunkp chunks = NULL;
   const int num = png_get_unknown_chunks(png_ptr, info_ptr, &chunks);
   int next = 0;

   /* Search for the fdAT with the given sequence number: */
   for (;; ++next)
   {
      next = apng_search(num, chunks, next, APNG_FIND_fdAT);

      if (next >= num) return 0;

      /* The chunk must be big enough for the sequence number, which is at the
       * start, and the sequence number must match the request:
       */
      if (chunks[next].size >= 4U && /* correct size */
          APNG_U32_RGB(chunks[next].data) == sqn) /* sequence number */
      {
         /* The sequence number matched and the size matched, get the data:
          */
         if (data_len_ptr != NULL)
            *data_len_ptr = chunks[next].size - 4U;

         if (data_ptr != NULL)
            *data_ptr = chunks[next].data + 4U;
         return true;
      }
   }
}
