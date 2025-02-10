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
apng_find_after(png_const_structp png_ptr, png_infop info_ptr,
   png_uint_32 after, int find_last, unsigned what)
{
   png_unknown_chunkp chunks = NULL;
   const int num = png_get_unknown_chunks(png_ptr, info_ptr, &chunks);
   png_uint_32 found = APNG_SQN_MISSING;

   if (find_last)
   {
      if ((what & (APNG_FIND_fcTL|APNG_FIND_fdAT)) ==
            (APNG_FIND_fcTL|APNG_FIND_fdAT))
         find_last = 2;
      else
         find_last = 1;
   }

   if (chunks != NULL && num > 0)
   {
      int i;
      int iRestart = num;

      for (i=0; i<num; ++i) switch (APNG_U32_RGB(chunks[i].name))
      {
         case APNG_fcTL:
            if ((what & APNG_FIND_fcTL) != 0U && chunks[i].size == 26U)
            {
               const png_uint_32 s = APNG_U32_RGB(chunks[i].data);

               if (s < 0x80000000U && s > after)
               {
                  if (find_last)
                  {
                     if (found == APNG_SQN_MISSING || s > found)
                        found = s;
                  }

                  else if (s < found)
                     found = s;
               }
            }
            break;

         case APNG_fdAT:
            if ((what & APNG_FIND_fdAT) != 0U && chunks[i].size >= 4U)
            {
               const png_uint_32 s = APNG_U32_RGB(chunks[i].data);

               if (s < 0x80000000U && s > after) switch (find_last)
               {
                  default: /* I.e. '2' above */
                     if (found == APNG_SQN_MISSING || s > found)
                        found = s;
                     break;

                  case 0:
                     if (s < found)
                        found = s;
                     break;

                  case 1:
                     /* With these settings the loop is trying to find the last
                      * fdAT in a continuous sequence starting with after+1U.
                      */
                     if (s == after+1U) /* next in sequence */
                     {
                        found = s;
                        after += 1U; /* seach for next */

                        /* This is efficient when chunks are in order or only
                         * slightly out of order.  When chunks are reversed this
                         * comes out as searching chunks[0..num) for each
                         * sequence number until one is not found.
                         */
                        if (iRestart < num)
                           i = iRestart-1; /* will be incremented by 'for' */
                     }

                     else /* s > after+1U */ if (i < iRestart)
                        iRestart = i;  /* used after 'after+1' found */

                     break;
               }
            }
            break;

         default:
            break;
      }
   }

   return found;
}
