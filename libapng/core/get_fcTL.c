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
apng_get_fcTL(png_const_structp png_ptr, png_infop info_ptr, png_uint_32 sqn,
   png_uint_32p width_ptr, png_uint_32p height_ptr,
   png_uint_32p x_offset_ptr, png_uint_32p y_offset_ptr,
   unsigned *delay_num_ptr, unsigned *delay_den_ptr,
   unsigned *dispose_op_ptr, unsigned *blend_op_ptr)
{
   png_unknown_chunkp chunks = NULL;
   const int num = png_get_unknown_chunks(png_ptr, info_ptr, &chunks);
   int next = 0;

   /* Search for the fcTL with the given sequence number: */
   for (;; ++next)
   {
      next = apng_search(num, chunks, next, APNG_FIND_fcTL);

      if (next >= num) return 0;

      /* The chunk must be exactly 26 bytes and the sequence number, which is
       * at the start, must match the request:
       */
      if (chunks[next].size == 26U && /* correct size */
          APNG_U32_RGB(chunks[next].data) == sqn) /* sequence number */
      {
         /* The sequence number matched and the size matched, get the data
          * and validate it:
          */
         const png_uint_32 width     = APNG_U32_RGB(chunks[next].data +  4U);
         const png_uint_32 height    = APNG_U32_RGB(chunks[next].data +  8U);
         const png_uint_32 x_offset  = APNG_U32_RGB(chunks[next].data + 12U);
         const png_uint_32 y_offset  = APNG_U32_RGB(chunks[next].data + 16U);
         const unsigned delay_num    = APNG_U16_RGB(chunks[next].data + 20U);
         const unsigned delay_den    = APNG_U16_RGB(chunks[next].data + 22U);
         const unsigned dispose_op   = chunks[next].data[24U];
         const unsigned blend_op     = chunks[next].data[25U];

         /* Validate the data, this is mostly a set of PNG unsigned integers
          * so this works:
          */
         if (((width|height|x_offset|y_offset) & 0x80000000U) == 0U &&
             dispose_op <= APNG_DISPOSE_OP_PREVIOUS &&
             blend_op <= APNG_BLEND_OP_OVER)
         {
            /* Valid: */
            *width_ptr = width;
            *height_ptr = height;
            *x_offset_ptr = x_offset;
            *y_offset_ptr = y_offset;
            *delay_num_ptr = delay_num;
            *delay_den_ptr = (delay_den > 0) ? delay_den : 100;
            *dispose_op_ptr = dispose_op;
            *blend_op_ptr = blend_op;

            return true;
         }
      }
   }
}
