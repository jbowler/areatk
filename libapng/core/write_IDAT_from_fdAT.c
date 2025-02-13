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

png_uint_32 APNGAPI
apng_write_IDAT_from_fdAT(png_structp png_ptr, png_infop info_ptr,
      png_uint_32 after, png_alloc_size_t *total_bytes_ptr)
{
   if (png_ptr == NULL || info_ptr == NULL)
      return APNG_SQN_EINVAL;

   png_alloc_size_t total_bytes = 0;
   bool overflow = false;

   if (APNG_SQN_IS_VALID(after))
   {
      png_unknown_chunkp chunks = NULL;
      const int num = png_get_unknown_chunks(png_ptr, info_ptr, &chunks);

      if (num > 0 && chunks != NULL) while (++after < APNG_SQN_INVALID)
      {
         png_uint_32 size = 0U;
         png_bytep data = NULL;

         if (apng_get_fdAT(png_ptr, info_ptr, after, &size, &data))
         {
            if (size > 0U)
            {
               /* This may overflow, if so just record that it did: */
               total_bytes += size;
               if (total_bytes < size) overflow = true;
               apng_write_chunk(png_ptr, APNG_IDAT_str, data, size);
            }
         }

         else
            break; /* missing sequence number or not fdAT */
      }
   }

   if (total_bytes_ptr != NULL)
      *total_bytes_ptr = overflow ? PNG_SIZE_MAX : total_bytes;

   return after;
}
