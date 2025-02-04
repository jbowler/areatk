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

void APNGAPI
apng_add_fdAT(png_const_structp png_ptr, png_infop info_ptr,
   png_uint_32p sqn_ptr, png_alloc_size_t data_len, png_const_bytep data)
{
   if (png_ptr != NULL && info_ptr != NULL && sqn_ptr != NULL && data != NULL)
   {
      png_uint_32 sqn = *sqn_ptr;
      png_unknown_chunk unknown;
      png_byte buffer[1024U-12U];

      memset(&unknown, 0, sizeof unknown);
      memset(buffer, 0, sizeof buffer);

      memcpy(unknown.name, APNG_fdAT_str, 5U);
      unknown.data = buffer;
      unknown.location = PNG_AFTER_IDAT;

      while (data_len > 0U && sqn < 0x80000000U)
      {
         png_alloc_size_t size = (sizeof buffer) - 4U;

         APNG_STORE_U32(buffer, 0U, sqn++);

         if (size > data_len) size = data_len;

         memcpy(buffer+4U, data, size);
         data += size;
         data_len -= size;

         unknown.size = size;

         png_set_unknown_chunks(png_ptr, info_ptr, &unknown, 1);
      }

      /* At this point sqn can be 0x80000000 but that will cause the png_error
       * that follows if it's an issue.
       */
      *sqn_ptr = sqn;

      if (data_len > 0U)
         png_error(png_ptr, "fdAT: too many APNG chunks");
   }

   else if (png_ptr != NULL)
      png_error(png_ptr, "png_add_fdAT bad parameters");
}
