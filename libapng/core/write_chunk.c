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

APNGPRIVATE void
apng_write_chunk(png_structp png_ptr, png_const_voidp name,
      png_const_bytep data, png_uint_32 len)
{
   png_write_chunk_start(png_ptr, name, len);

   if (len > 0)
      png_write_chunk_data(png_ptr, data, len);

   png_write_chunk_end(png_ptr);
}
