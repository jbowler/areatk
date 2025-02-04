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
apng_add_fcTL(png_const_structp png_ptr, png_infop info_ptr,
   int before_IDAT, png_uint_32p sqn_ptr,
   png_uint_32 width, png_uint_32 height,
   png_uint_32 x_offset, png_uint_32 y_offset,
   unsigned delay_num, unsigned delay_den,
   unsigned dispose_op, unsigned blend_op)
{
   if (png_ptr != NULL && info_ptr != NULL && sqn_ptr != NULL)
   {
      png_uint_32 sqn = *sqn_ptr;

      if (((sqn|width|height|x_offset|y_offset) & 0x80000000U) == 0U &&
          dispose_op <= APNG_DISPOSE_OP_PREVIOUS &&
          blend_op <= APNG_BLEND_OP_OVER &&
          (!before_IDAT || sqn == 0U))
      {
         png_byte fcTL[26];
         png_unknown_chunk unknown;

         APNG_STORE_U32(fcTL,  0U, sqn);
         APNG_STORE_U32(fcTL,  4U, width);
         APNG_STORE_U32(fcTL,  8U, height);
         APNG_STORE_U32(fcTL, 12U, x_offset);
         APNG_STORE_U32(fcTL, 16U, y_offset);
         APNG_STORE_U16(fcTL, 20U, delay_num);
         APNG_STORE_U16(fcTL, 22U, delay_den);
         fcTL[24U] = dispose_op & 0xFFU;
         fcTL[25U] = blend_op & 0xFFU;

         memset(&unknown, 0, sizeof unknown);

         memcpy(unknown.name, APNG_fcTL_str, 5U);
         unknown.data = fcTL;
         unknown.size = 26U;
         unknown.location =
            (before_IDAT && sqn == 0U) ? PNG_HAVE_IHDR : PNG_AFTER_IDAT;

         png_set_unknown_chunks(png_ptr, info_ptr, &unknown, 1);
         *sqn_ptr = sqn + 1U;
      }

      else if ((sqn & 0x80000000U) != 0U)
         png_error(png_ptr, "fcTL: too many APNG chunks");

      else
         png_error(png_ptr, "fcTL invalid");
   }

   else if (png_ptr != NULL)
      png_error(png_ptr, "png_add_fcTL bad parameters");
}
