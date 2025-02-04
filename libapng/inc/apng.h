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

#ifndef APNG_H
#define APNG_H

#include <png.h>

#ifndef APNGAPI
#  define APNGAPI
#endif

/* Definitions from the W3C specification. */
#define APNG_DISPOSE_OP_NONE       0 /* canvas remains the same */
#define APNG_DISPOSE_OP_BACKGROUND 1 /* canvas set to alpha 0 */
#define APNG_DISPOSE_OP_PREVIOUS   2 /* canvas restored to prior canvas */

#define APNG_BLEND_OP_SOURCE 0 /* Replace current pixel values */
#define APNG_BLEND_OP_OVER   1 /* Compose current values, including alpha */

/* CORE READ API {#core-read} */
/* {#apng-read-enable} */
int APNGAPI apng_read_enable(png_structrp png_ptr);
   /* Sets up 'unknown' handling for the APNG chunks and returns true if this
    * succeeds.
    *
    * Returns false if this build of libpng does not support the unknown chunk
    * handling mechanism.  Standard builds of libpng do have the required
    * support.
    *
    * The function adds the three APNG chunks to the list of unknown chunks
    * using the libpng function png_set_keep_unknown_chunks and marks them as
    * always to be saved (PNG_HANDLE_CHUNK_ALWAYS).  The default handling of
    * unknown chunks (by default discard) is not changed.
    *
    * Call this after setting up application unknown handling or do the same
    * thing in application unknown handling (in which case this need not be
    * called).  png_get_acTL checks for the correct handling and will call
    * png_error if it is not correct.
    */

/* {#apng-get-acTL} */
bool APNGAPI apng_get_acTL(png_const_structp png_ptr, png_infop info_ptr,
      png_uint_32p num_frames_ptr, png_uint_32p num_plays_ptr);
   /* Returns true if a valid acTL chunk exists in the info_struct pointed to by
    * info_ptr and false otherwise.  If the num_ ptr values are non-NULL it also
    * fills those values in if true is returned.
    */

/* {#apng-get-fcTL} */
bool APNGAPI apng_get_fcTL(png_const_structp png_ptr, png_infop info_ptr,
      png_uint_32 sqn, png_uint_32p width_ptr, png_uint_32p height_ptr,
      png_uint_32p x_offset_ptr, png_uint_32p y_offset_ptr,
      unsigned *delay_num_ptr, unsigned *delay_den_ptr,
      unsigned *dispose_op_ptr, unsigned *blend_op_ptr);
   /* Returns true if a valid fcTL chunk with the given sequence number exists
    * in the info_struct pointed to by info_ptr and false otherwise.  If the
    * num_ ptr values are non-NULL it also fills those values in if true is
    * returned.
    *
    * The delay denominator result will always be greater than 0, the dipose and
    * blend operations are checked for validity.
    *
    * There is **no** check on the fcTL chunk location; this allows the caller
    * to move fcTL chunks which are before IDAT but do not have a sequence
    * number of 0 to be after IDAT.
    */

/* {#apng-get-fdAT} */
bool APNGAPI apng_get_fdAT(png_const_structp png_ptr, png_infop info_ptr,
      png_uint_32 sqn, png_uint_32p data_len_ptr, png_const_bytep *data_ptr);
   /* Returns true if a valid fdAT chunk with the given sequence number exists
    * in the info_struct pointed to by info_ptr and false otherwise.
    *
    * *data_len_ptr and *data_ptr are filled in with the "IDAT" data in the
    * chunk; (*data_ptr)[*data_len_ptr].  The data may be of zero length.
    */

/* {#apng-find-after} */
#  define APNG_FIND_acTL 0x01U
#  define APNG_FIND_fcTL 0x02U
#  define APNG_FIND_fdAT 0x04U

/* {#apng-find-after} */
png_uint_32 APNGAPI apng_find_after(png_const_structp png_ptr,
      png_infop info_ptr, png_uint_32 after, int find_last, unsigned what);
   /* Returns a sequence number greater than 'after' or
    * APNG_INVALID_SEQUENCE_NUMBER if no chunk matching the conditions is found.
    * The conditions depend on 'what' which takes the following values:
    *
    *    PNG_FIND_fcTL:
    *       !find_last: returns the next fcTL sequence number; the lowest
    *          fcTL sequence number greater than 'after'.
    *
    *       find_last: returns the sequence number of the last fcTL in the
    *          info_struct so long as it is greater than 'after'.  The highest
    *          fcTL sequence number.
    *
    *    PNG_FIND_fdAT:
    *       !find_last: returns the next fdAT sequence number; the lowest
    *          fdAT sequence number greater than 'after'.
    *
    *       find_last: returns the last fdAT sequence number in a continuous
    *          sequence starting at 'after+1'; all sequence numbers from
    *          'after+1' to the returned number inclusive are present in the
    *          APNG stream.
    *
    *    PNG_FIND_fcTL | PNG_FIND_fdAT:
    *       !find_last: returns the next fdAT or fcTL sequence number; the
    *          lowest sequence number greater than 'after'.
    *
    *       find_last: behaves as with PNG_FIND_fcTL except the sequence
    *          numbers of both chunk types are considered:  Returns the highest
    *          sequence number in the APNG or PNG_INVALID_SEQUENCE_NUMBER if
    *          there is no sequence number higher than 'after'.
    *
    * This API exists to allow a more rapid check for a valid APNG stream by
    * searching in the after-IDAT info_struct for frames (PNG_FIND_fcTL) then
    * the frame data (PNG_FIND_fdAT).  The combination of both allows any gaps
    * in the sequence numbers to be skipped without checking for every
    * intervening (absent) sequence number.
    */

/* CORE WRITE API {#core-write} */
/* {#apng-set-acTL} */
void APNGAPI apng_set_acTL(png_const_structp png_ptr, png_infop info_ptr,
      png_uint_32 num_frames, png_uint_32 num_plays);
   /* Creates an acTL chunk.  This may be called only once and must be called on
    * the before-IDAT info_struct.
    */

/* {#apng-add-fcTL} */
void APNGAPI apng_add_fcTL(png_const_structp png_ptr, png_infop info_ptr,
      int before_IDAT, png_uint_32p sqn_ptr,
      png_uint_32 width, png_uint_32 height,
      png_uint_32 x_offset, png_uint_32 y_offset,
      unsigned delay_num, unsigned delay_den,
      unsigned dispose_op, unsigned blend_op);
   /* Adds an fcTL chunk.  If before_IDAT is set *sqn must be 0 and the
    * info_ptr must point to the before-IDAT info_struct, otherwise info_ptr
    * must point to a separate after-IDAT info_struct.
    *
    * *sqn_ptr is updated (by adding 1).
    */

/* {#apng-add-fdAT} */
void APNGAPI apng_add_fdAT(png_const_structp png_ptr, png_infop info_ptr,
      png_uint_32p sqn_ptr, png_alloc_size_t data_len, png_const_bytep data);
   /* Adds fdAT chunks.  These must be created in the after-IDAT
    * info_struct.
    *
    * *sqn_ptr must be the sequence number of the first fdAT to be added.  It is
    * incremented as required.
    */
#endif /* APNG_H */
