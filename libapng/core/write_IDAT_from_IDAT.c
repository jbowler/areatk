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
apng_write_IDAT_from_IDAT(png_structp png_ptr, png_infop info_ptr,
      png_alloc_size_t *total_bytes_ptr)
{
   if (png_ptr == NULL || info_ptr == NULL)
      return false;

   png_unknown_chunkp chunks = NULL;
   const int num = png_get_unknown_chunks(png_ptr, info_ptr, &chunks);

   png_alloc_size_t total_bytes = 0;
   bool overflow = false;

   /* Find the first IDAT with a size greater than 0: */
   int i;

   if (chunks != NULL) for (i=0; i<num; ++i)
   {
      i = apng_search(num, chunks, i, APNG_FIND_IDAT);

      if (i >= num) break;

      png_uint_32 size = chunks[i].size;

      if (size > 0U)
      {
         apng_write_chunk(png_ptr, APNG_IDAT_str, chunks[i].data, size);

         /* This may overflow, if so just record that it did: */
         total_bytes += size;
         if (total_bytes < size) overflow = true;
      }
   }

   if (total_bytes_ptr != NULL)
      *total_bytes_ptr = overflow ? PNG_SIZE_MAX : total_bytes;

   /* The result indicates if an IDAT was written. */
   return overflow || total_bytes > 0U;
}
