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

/* Test the APNG core read functionality.
 *
 * The code is a compilable example of the code in the documentation of the
 * [library](../doc/LIBRARY.md#core-read).
 *
 * The file to be tested is read from stdin.
 */
#include <stdlib.h>
#include <stdio.h>

#include "../inc/apng.h"

typedef struct
{
   bool warnings;
} error_data;

static void
handle_error(png_structp, png_const_charp msg)
{
   fprintf(stderr, "ERROR: %s\n", msg);
   /* Doing this allows the program to avoid setting up any error handling;
    * without an exit here libpng will call abort(1).
    */
   exit(1);
}

static void
handle_warning(png_structp, png_const_charp msg)
{
   fprintf(stderr, "WARNING: %s\n", msg);
}

struct fcTL
{
   png_uint_32 sqn, width, height, x_offset, y_offset;
   unsigned delay_num, delay_den, dispose_op, blend_op;
};

static bool
get_fcTL(png_const_structrp png_ptr, png_inforp info_ptr, png_uint_32 sqn,
      struct fcTL *fcTL)
{
   fcTL->sqn = sqn;

   return apng_get_fcTL(png_ptr, info_ptr, sqn, &fcTL->width, &fcTL->height,
            &fcTL->x_offset, &fcTL->y_offset,
            &fcTL->delay_num, &fcTL->delay_den,
            &fcTL->dispose_op, &fcTL->blend_op);
}

static const char *dispose_of(int dispose)
{
   switch (dispose)
   {
      case APNG_DISPOSE_OP_NONE:       return "none";
      case APNG_DISPOSE_OP_BACKGROUND: return "clear";
      case APNG_DISPOSE_OP_PREVIOUS:   return "previous";
      default:                         return "INVALID";
   }
}

static const char *blend_of(int dispose)
{
   switch (dispose)
   {
      case APNG_BLEND_OP_SOURCE: return "replace";
      case APNG_BLEND_OP_OVER:   return "Porter-Duff(over)";
      default:                   return "INVALID";
   }
}

static void
print_fcTL(const struct fcTL *fcTL, png_uint_32 frame_count)
{
   printf(
      "INFO: fcTL[%u(%u)]: (%ux%u)+(%u,%u) DISPOSE(%s(%u)) BLEND(%s(%u)) DELAY %gs\n",
      frame_count, fcTL->sqn,
      fcTL->width, fcTL->height,
      fcTL->x_offset, fcTL->y_offset,
      dispose_of(fcTL->dispose_op), fcTL->dispose_op,
      blend_of(fcTL->blend_op), fcTL->blend_op,
      (1.*fcTL->delay_num)/fcTL->delay_den);
}

int main(void)
{
   error_data ed;
   ed.warnings = 1;

   /* (!) Create a PNG read structure: */
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, &ed,
         handle_error, handle_warning);

   if (png_ptr == NULL)
   {
      fprintf(stderr, "ERROR: png_create_read_struct failed\n");
      exit(1);
   }

   /* Set the IO handling. */
   png_init_io(png_ptr, stdin);

   /* (!) Set application unknown chunk handling if required. */
   /* This avoids needing to read an image and allows the IDAT chunks to be read
    * from the info-after-IDAT png_infop.  This is convenient for low level
    * handling of APNG.
    *
    * Using a C string here is lazy and only works reliably on ASCII or UTF-8
    * systems but it is very convenient.
    */
   static const png_byte IDAT[] = "IDAT";
   png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, IDAT, 1);

   /* (!) enable APNG processing */
   if (!apng_read_enable(png_ptr))
   {
      fprintf(stderr, "ERROR: APNG not supported\n");
      exit(1);
   }

   /* (!) Call png_read_info(png_ptr, pre_IDAT_info_ptr) */
   png_infop pre_IDAT_info_ptr = png_create_info_struct(png_ptr);

   if (pre_IDAT_info_ptr == NULL) /* this should never happen */
      png_error(png_ptr, "create info-before-IDAT");

   png_read_info(png_ptr, pre_IDAT_info_ptr);

   /* (!) Call apng_get_acTL(png_ptr, pre_IDAT_info_ptr, ...); */
   png_uint_32 num_frames = 0, num_plays = 1;

   if (apng_get_acTL(png_ptr, pre_IDAT_info_ptr, &num_frames, &num_plays))
      printf("INFO: this is an APNG with %u frame%s x %u loop%s\n",
            num_frames, num_frames != 1 ? "s" : "",
            num_plays, num_plays != 1 ? "s" : "");

   else
      printf("INFO: no valid acTL chunk: this is not an APNG\n");

   /* This test code continues to look for fcTL and fdAT if acTL is not found,
    * this is useful for detecting errors.
    */

   /* (!) if so call apng_get_fcTL to see whether the IDAT needs to be retained
    * for a looping (num_plays != 1) APNG and to determine the first fdAT
    * sequence number.
    *
    * libapng does not verify all of the rules fcTL[0] has to obey at this point
    * so this may output bogus information for a bogus APNG.
    */
   struct fcTL fcTL;

   if (get_fcTL(png_ptr, pre_IDAT_info_ptr, 0U, &fcTL))
   {
      printf("INFO: fcTL[0] BEFORE IDAT\n");
      print_fcTL(&fcTL, 0U);
      fcTL.sqn++;
   }

   else
      printf("INFO: fcTL[0] AFTER IDAT\n");

   /* (!) Read the main image
    *
    * Because 'IDAT' have been marked as unknown this does not require any
    * work at all; libpng does it all at the next step.
    *
    * (!) png_read_end(png_ptr, post_IDAT_info_ptr)
    */
   png_infop post_IDAT_info_ptr = png_create_info_struct(png_ptr);

   if (post_IDAT_info_ptr == NULL) /* this should never happen */
      png_error(png_ptr, "create info-after-IDAT");

   png_read_end(png_ptr, post_IDAT_info_ptr);

   /* (!) **Animation loop** to find all the frames in the animation.
    *
    * In this code 'sqn' can overflow a **PNG four-byte integer** in a very long
    * APNG, however the code is safe because a png_uint_32 accomodates the
    * overflow and the apng_get_ functions below will fail on an over-large
    * sqn (0x80000000 and above.)
    */
   png_uint_32 sqn = fcTL.sqn;    /* 0 or 1: updated for each fcTL or fdAT */
   png_uint_32 frame_count = sqn; /* incremented each fcTL */
   png_uint_32 fcTL_sqn = sqn;    /* updated for each fcTL */

   while (get_fcTL(png_ptr, post_IDAT_info_ptr, sqn, &fcTL))
   {
      print_fcTL(&fcTL, frame_count);
      fcTL_sqn = fcTL.sqn;

      /* (!) **Frame loop** to read all the `fdAT` chunks in this frame */
      png_uint_32 num_bytes;
      png_bytep frame_data;

      while (apng_get_fdAT(png_ptr, post_IDAT_info_ptr, ++sqn,
               &num_bytes, &frame_data))
         printf("INFO: fdAT[%u] data(%p)[%u] fcTL[%u(%u)]\n", sqn,
               frame_data, num_bytes, frame_count, fcTL_sqn);

      ++frame_count;
      /* Try for fcTL[sqn] */
   }

   /* End of the loop: sqn is one beyond the last sequence number found */
   printf( "INFO: APNG end fcTL+fdAT[%u chunk%s]: %u frame%s; last: %u\n",
         sqn, sqn > 1 ? "s" : "", frame_count, frame_count > 1 ? "s" : "",
         fcTL_sqn);
   return 0;
}
