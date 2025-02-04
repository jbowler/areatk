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

APNGPRIVATE int/*index*/
apng_search(int num, png_unknown_chunkp chunks, int next, unsigned what)
{
   if (chunks != NULL && num > 0 && next >= 0) for (; next < num; ++next)
      switch (APNG_U32_RGB(chunks[next].name))
      {
         case APNG_acTL:
            if (what & APNG_FIND_acTL) return next;
            continue;

         case APNG_fcTL:
            if (what & APNG_FIND_fcTL) return next;
            continue;

         case APNG_fdAT:
            if (what & APNG_FIND_fdAT) return next;
            continue;

         default:
            continue;
      }

   return num;
}
