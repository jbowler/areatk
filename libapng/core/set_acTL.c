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
apng_set_acTL(png_const_structp png_ptr, png_infop info_ptr,
      png_uint_32 num_frames, png_uint_32 num_plays)
{
   if (((num_frames | num_plays) & 0x80000000U) == 0U && num_frames > 0U)
   {
      png_byte acTL[8];
      png_unknown_chunk unknown;

      APNG_STORE_U32(acTL, 0U, num_frames);
      APNG_STORE_U32(acTL, 4U, num_plays);

      memset(&unknown, 0, sizeof unknown);
      memcpy(unknown.name, APNG_acTL_str, 5U);
      unknown.data = acTL;
      unknown.size = 8U;
      unknown.location = PNG_HAVE_IHDR;

      png_set_unknown_chunks(png_ptr, info_ptr, &unknown, 1);
   }

   else
      png_error(png_ptr, "acTL invalid");
}
