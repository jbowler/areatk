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

/* Demonstrate the APNG core functionality.
 *
 * This is a modified version of core_read.c which writes an individual frame
 * from the input APNG to stdout.  It preserves the chunks from the original PNG
 * and writes them to each file.
 *
 * The syntax of the program is:
 *
 *    test-core APNG-file frame-number PNG-file
 *
 * The frame number is an integer and -1 indicates the static (IDAT) image.
 */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "../inc/apng.h"

typedef struct
{
   const char *file_name;
   bool warnings;
} error_data;

static void
handle_error(png_structp png_ptr, png_const_charp msg)
{
   const error_data *ped = png_get_error_ptr(png_ptr);

   if (ped == NULL)
      printf("INTERNAL ERROR: no error data (%s)\n", msg);

   else
      printf("%s: ERROR: %s\n",
            ped->file_name ? ped->file_name : "<null>", msg);

   /* Doing this allows the program to avoid setting up any error handling;
    * without an exit here libpng will call abort(1).
    */
   exit(1);
}

static void
handle_warning(png_structp png_ptr, png_const_charp msg)
{
   const error_data *ped = png_get_error_ptr(png_ptr);

   if (ped == NULL)
      printf("INTERNAL ERROR: no error data (%s)\n", msg);

   else if (ped->warnings)
      printf("%s: WARNING: %s\n",
            ped->file_name ? ped->file_name : "<null>", msg);
}

/* fcTL HANDLING */
struct fcTL
{
   png_uint_32 sqn, width, height, x_offset, y_offset;
   unsigned delay_num, delay_den, dispose_op, blend_op;
};

static bool
get_fcTL(png_const_structp png_ptr, png_infop info_ptr, png_uint_32 sqn,
      struct fcTL *fcTL)
{
   fcTL->sqn = sqn;

   return sqn < 0x80000000U &&
      apng_get_fcTL(png_ptr, info_ptr, sqn, &fcTL->width, &fcTL->height,
            &fcTL->x_offset, &fcTL->y_offset,
            &fcTL->delay_num, &fcTL->delay_den,
            &fcTL->dispose_op, &fcTL->blend_op);
}

static const char *dispose_of(const struct fcTL *fcTL_ptr)
{
   switch (fcTL_ptr->dispose_op)
   {
      case APNG_DISPOSE_OP_NONE:       return "none";
      case APNG_DISPOSE_OP_BACKGROUND: return "clear";
      case APNG_DISPOSE_OP_PREVIOUS:   return "previous";
      default:                         return "INVALID";
   }
}

static const char *blend_of(const struct fcTL* fcTL_ptr)
{
   switch (fcTL_ptr->blend_op)
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
      dispose_of(fcTL), fcTL->dispose_op,
      blend_of(fcTL), fcTL->blend_op,
      (1.*fcTL->delay_num)/fcTL->delay_den);
}

/* IHDR handling */
struct IHDR
{
   png_uint_32 width, height;
   int bit_depth, color_type, interlace_method;
   int compression_method, filter_method;
};

static void
write_IDAT_chunk(png_structp write_ptr, png_const_bytep data, png_uint_32 len)
{
   const png_byte IDAT[] = "IDAT";

   png_write_chunk_start(write_ptr, IDAT, len);

   if (len > 0)
      png_write_chunk_data(write_ptr, data, len);

   png_write_chunk_end(write_ptr);
}

static png_uint_32
write_IDAT_data(png_structp write_ptr, png_const_structp png_ptr,
      png_infop info_ptr)
{
   /* Write the IDAT chunks stored as unknown in info_ptr: */
   png_unknown_chunkp chunks = NULL;
   const int num = png_get_unknown_chunks(png_ptr, info_ptr, &chunks);
   int next = 0;
   png_uint_32 count = 0;

   for (; next<num; ++next)
   {
      if (memcmp("IDAT", chunks[next].name, 4) == 0)
      {
         write_IDAT_chunk(write_ptr, chunks[next].data, chunks[next].size);
         ++count;
      }
   }

   return count;
}

static png_uint_32
write_fdAT_data(png_structp write_ptr, png_const_structp png_ptr,
      png_infop info_ptr, png_uint_32 sqn /*frame sqn*/)
{
   png_uint_32 num_bytes;
   png_bytep frame_data;
   png_uint_32 count = 0;

   while (apng_get_fdAT(png_ptr, info_ptr, ++sqn, &num_bytes, &frame_data))
   {
      write_IDAT_chunk(write_ptr, frame_data, num_bytes);
      ++count;
   }

   return count;
}

static bool
write_output(const char *file_name, png_structp png_ptr,
      png_infop pre_IDAT_info_ptr, png_infop post_IDAT_info_ptr,
      const struct IHDR *IHDR, const struct fcTL *fcTL, long frame_index,
      bool from_fdAT)
{
   /* The IHDR and any oFFs chunk need to be reset for this frame: */
   png_set_IHDR(png_ptr, pre_IDAT_info_ptr, fcTL->width, fcTL->height,
         IHDR->bit_depth, IHDR->color_type, IHDR->interlace_method,
         IHDR->compression_method, IHDR->filter_method);

   png_set_oFFs(png_ptr, pre_IDAT_info_ptr,
         fcTL->x_offset, fcTL->y_offset, 0/*pixels*/);

   /* Text chunks can be added but be aware that libpng 1.6 has a bug with text
    * chunks in that it marks them as "written" in png_write_info and this
    * prevents them being written again using the same info_struct.  Also text
    * chunks cannot be removed; none of the "multiple" chunks can be cleared.
    *
    * Consequently while this code guarantees to retain all valid chunks and
    * does not require processing the ancillary chunks as "unknown" (which would
    * remove the validity checks) it has limitations when run in a loop to
    * extract all frames.
    */
   png_text apng_text;
   apng_text.compression = -1; /* tEXt: No compression */
   char key[] = "APNG:fcTL";
   apng_text.key = key;

   char buffer[256];

   if (frame_index < 0) /* dummy fcTL */
      apng_text.text_length = snprintf(buffer, sizeof buffer, "[STATIC IMAGE]");
   else /* valid fcTL */
      apng_text.text_length = snprintf(buffer, sizeof buffer,
            "fcTL[%ld%s] SEQUENCE=%u DELAY=%gs DISPOSE=%s[%u] BLEND=%s[%u]",
         frame_index,
         fcTL->sqn == APNG_SQN_STATIC_IMAGE ? " [static image]" : "", fcTL->sqn,
         fcTL->delay_num / (fcTL->delay_den == 0 ? 100. : fcTL->delay_den),
         dispose_of(fcTL), fcTL->dispose_op, blend_of(fcTL), fcTL->blend_op);

   apng_text.text = buffer;
   apng_text.itxt_length = 0;
   apng_text.lang = NULL;
   apng_text.lang_key = NULL;

   png_set_text(png_ptr, pre_IDAT_info_ptr, &apng_text, 1);

   /* Create a PNG write structure: */
   error_data output_error;
   output_error.file_name = file_name;
   output_error.warnings = 1;

   png_structp write_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
         &output_error, handle_error, handle_warning);

   if (write_ptr == NULL)
      png_error(png_ptr, "ERROR: png_create_write_struct failed\n");

   /* (!) Set the chunk handling for write. */
   /* The acTL, fcTL, fdAT chunks must not be written, the saved IDAT chunks may
    * not be written because they get written before PLTE due to a bug in libpng
    * 1.6:
    */
   {
      const png_byte chunks[] = "acTL\0fcTL\0fdAT\0IDAT";
      png_set_keep_unknown_chunks(write_ptr, PNG_HANDLE_CHUNK_NEVER, chunks, 3);
   }

   /* Set the output IO handling. */
   FILE *output;
   {
      output = fopen(file_name, "wb");
      if (output == NULL)
         {
            printf("ERROR: %s: write open failed: %s\n", file_name,
                  strerror(errno));
            exit(1);
         }

      png_init_io(write_ptr, output);
   }

   /* Now write the info before IDAT.
    */
   png_write_info(write_ptr, pre_IDAT_info_ptr);

   /* Now the IDAT chunks, this is done using something similar to the "frame
    * loop" in test/core_read.c and the "core" documentation:
    */
   png_uint_32 count;
   if (from_fdAT)
      count = write_fdAT_data(write_ptr, png_ptr, post_IDAT_info_ptr,
            fcTL->sqn);
   else
      count = write_IDAT_data(write_ptr, png_ptr, pre_IDAT_info_ptr);

   png_write_info(write_ptr, post_IDAT_info_ptr);

   /*
    * png_write_end will fail if IDAT was written as "unknown".  IEND also
    * cannot be added to the unknown chunks because it is of zero length.
    *
    * So write it by hand:
    */
   const png_byte IEND[] = "IEND";
   png_write_chunk_start(write_ptr, IEND, 0U);
   png_write_chunk_end(write_ptr);
   if (fflush(output))
   {
      printf("ERROR: %s: flush failed: %s\n", file_name, strerror(errno));
      exit(1);
   }
   fclose(output);

   printf("INFO: WROTE '%s' with %u IDAT chunks from %s\n",
         output_error.file_name, count, from_fdAT ? "fdAT" : "IDAT");
   return count > 0;
}

int main(int argc, const char * const *argv)
{
   if (argc != 4)
   {
      printf("usage: test-core input-APNG-file frame output-PNG-file\n");
      exit(1);
   }

   /* Read the arguments: */
   error_data input_error;
   input_error.file_name = *++argv;
   input_error.warnings = 1;

   long frame_index = atol(*++argv);

   ++argv; /* The output file name */

   /* (!) Create a PNG read structure: */
   png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
         &input_error, handle_error, handle_warning);

   if (png_ptr == NULL)
   {
      printf("ERROR: png_create_read_struct failed\n");
      exit(1);
   }

   /* Set the input IO handling. */
   {
      FILE *input = fopen(input_error.file_name, "rb");
      if (input == NULL)
         {
            printf("ERROR: %s: read open failed: %s\n", input_error.file_name,
                  strerror(errno));
            exit(1);
         }

      png_init_io(png_ptr, input);
   }

   /* (!) Set application unknown chunk handling if required. */
   /* IDAT chunks are made "unknown" here to give access to the compressed image
    * data of the static image, which may also be the first fcTL.
    */
   {
      const png_byte IDAT[] = "IDAT";
      png_set_keep_unknown_chunks(png_ptr, PNG_HANDLE_CHUNK_ALWAYS, IDAT, 1);
   }

   /* (!) enable APNG processing */
   if (!apng_read_enable(png_ptr))
   {
      printf("ERROR: APNG not supported\n");
      exit(1);
   }

   /* (!) Call png_read_info(png_ptr, pre_IDAT_info_ptr).  The IDAT chunks end
    * up in this png_info.
    */
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

   if (frame_index < -1)
      exit(0);

   /* This test code continues to look for fcTL and fdAT if acTL is not found,
    * this is useful for detecting errors.
    */

   /* (!) Call apng_get_fcTL to see whether the IDAT needs to be retained
    * for a looping (num_plays != 1) APNG and to determine the first fdAT
    * sequence number.
    *
    * libapng does not verify all of the rules fcTL[0] has to obey at this point
    * so this may output bogus information for a bogus APNG.
    */
   struct IHDR IHDR;
   png_get_IHDR(png_ptr, pre_IDAT_info_ptr,
         &IHDR.width, &IHDR.height, &IHDR.bit_depth, &IHDR.color_type,
         &IHDR.interlace_method, &IHDR.compression_method, &IHDR.filter_method);

   struct fcTL fcTL;

   if (get_fcTL(png_ptr, pre_IDAT_info_ptr, 0U, &fcTL))
   {
      printf("INFO: fcTL[0] BEFORE IDAT\n");
      print_fcTL(&fcTL, 0U);
   }

   else
   {
      printf("INFO: fcTL[0] AFTER IDAT\n");
      fcTL.sqn = APNG_SQN_STATIC_IMAGE;
      fcTL.width = IHDR.width;
      fcTL.height = IHDR.height;
      fcTL.x_offset = fcTL.y_offset = 0;
      fcTL.delay_num = fcTL.delay_den = 0;
      fcTL.dispose_op = APNG_DISPOSE_OP_BACKGROUND;
      fcTL.blend_op = APNG_BLEND_OP_OVER;
   }

   /* (!) Read the main image
    *
    * Because 'IDAT' have been marked as unknown this does not require any
    * work at all; libpng has done it all already.
    *
    * (!) png_read_end(png_ptr, post_IDAT_info_ptr)
    */
   png_infop post_IDAT_info_ptr = png_create_info_struct(png_ptr);

   if (post_IDAT_info_ptr == NULL) /* this should never happen */
      png_error(png_ptr, "create info-after-IDAT");

   png_read_end(png_ptr, post_IDAT_info_ptr);

   bool wrote_image = false;

   /* Write the static image to the output if requested: */
   if (frame_index < 0 || (frame_index == 0 && fcTL.sqn == 0U))
   {
      printf("INFO: FOUND static image [%u x %u]\n", IHDR.width, IHDR.height);

      wrote_image = write_output(*argv, png_ptr,
            pre_IDAT_info_ptr, post_IDAT_info_ptr, &IHDR, &fcTL, frame_index,
            false/*!from fdAT; from IDAT*/);
   }

   /* (!) **Animation loop** to find all the frames in the animation.
    *
    * In this code 'sqn' can overflow a **PNG four-byte integer** in a very long
    * APNG, however the code is safe because a png_uint_32 accomodates the
    * overflow and the apng_get_ functions below will fail on an over-large
    * sqn (0x80000000 and above.)
    */
   png_uint_32 sqn = fcTL.sqn; /* incremented for each fcTL or fdAT */
   long fcTL_index = (sqn == 0U) ? 0 : -1; /* incremented for each fcTL */

   while (get_fcTL(png_ptr, post_IDAT_info_ptr, ++sqn, &fcTL))
   {
      const png_uint_32 fcTL_sqn = fcTL.sqn;

      print_fcTL(&fcTL, ++fcTL_index);

      /* (!) **Frame loop** to read all the `fdAT` chunks in this frame */
      size_t total_bytes = 0U;
      png_uint_32 num_bytes;
      png_bytep frame_data;
      png_uint_32 num_fdATs = 0U;

      while (apng_get_fdAT(png_ptr, post_IDAT_info_ptr, ++sqn,
               &num_bytes, &frame_data))
      {
         printf("INFO: fdAT[%u(%u)] data(%p)[%u] fcTL[%ld(%u)]\n", num_fdATs++,
               sqn, frame_data, num_bytes, fcTL_index, fcTL_sqn);
         total_bytes += num_bytes; /* may overflow on some systems */
      }

      printf("INFO: FRAME[%ld] with %u fdATs holding %zu total bytes of IDAT\n",
            fcTL_index, num_fdATs, total_bytes);

      if (frame_index == fcTL_index)
      {
         if (num_fdATs == 0U)
            printf("ERROR: No fdATs found\n");
         else if (total_bytes == 0U)
            printf("ERROR: fdAT[%u] holds zero bytes\n", num_fdATs);
         else if (wrote_image)
            png_error(png_ptr, "INTERNAL ERROR");
         else
         {
            wrote_image = write_output(*argv, png_ptr,
                  pre_IDAT_info_ptr, post_IDAT_info_ptr,
                  &IHDR, &fcTL, fcTL_index, true/*from fdAT*/);
         }
      }

      ++fcTL_index;
      /* Try for fcTL[sqn] */
   }

   /* End of the loop: sqn is one beyond the last sequence number found */
   printf( "%s: APNG end fcTL+fdAT[%u chunk%s]: %ld frame%s; last: %u\n",
         wrote_image ? "ERROR" : "INFO", sqn,
         sqn > 1 ? "s" : "", frame_index, frame_index > 1 ? "s" : "", fcTL.sqn);

   return wrote_image ? 0 : 2;
}
