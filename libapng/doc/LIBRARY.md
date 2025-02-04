# LIBRARY **libapng** {#library}

The AREATK library is used by including the `apng.h` C header file in code
which needs to use the library functionality.  When this header file is
included the `png.h` header from [libpng](#libpng) is also included.  Consult
the [HOW TO](#lib-use) section for more discussion of when to use `apng.h` as
opposed to `png.h`.

- [CORE FUNCTIONS](#lib-core)
- [HOW TO USE](#lib-use)
- [READING APNG CHUNKS](#core-read)

## CORE FUNCTIONS {#lib-core}

AREATK core functions allow access to the APNG chunks using functions which
follow the traditional [libpng](#libpng) chunk API.  APIs to read chunks
from PNG files have the general name `png_get_cHNk` where "cHNk" is the PNG
chunk name.  APIs to write chunks have the corresponding general name
`png_set_cHNk`.

Detailed documentation of the individual functions is in the
[function definitions](functions.md#functions-core) section of this document.

The functionality provided for reading is:

- [apng\_read\_enable(png\_ptr)](
    functions.md#apng-read-enable)
- [apng\_get\_acTL(png\_ptr, info\_ptr, &num\_frames &num\_plays)](
    functions.md#apng-get-acTL)
- [apng\_get\_fcTL(png\_ptr, info\_ptr, sequence\_number, &<fcTL parameters>)](
    functions.md#apng-get-fcTL)
- [apng\_get\_fdAT(png\_ptr, info\_ptr, sequence\_number, &num\_bytes, &frame\_data)](
    functions.md#apng-get-fdAT)

The functionality provided for writing:

- [apng\_set\_acTL(png\_ptr, info\_ptr, num\_frames num\_plays)](
    functions.md#apng-set-acTL)
- [apng\_add\_fcTL(png\_ptr, info\_ptr, sequence\_number, <fcTL parameters>)](
    functions.md#apng-add-fcTL)
- [apng\_add\_fdAT(png\_ptr, info\_ptr, sequence\_number, num\_bytes, frame\_data)](
    functions.md#apng-add-fdAT)

## HOW TO USE {#lib-use}

### LIBAPNG {#libapng-use}

    #include <apng.h> /* Do this **INSTEAD OF** #include <png.h> */

Do NOT `#include` `png.h` before `apng.h` if you are building against the
libapng library.

Building against the library will give you compatibility with all versions of
libpng-1.6 including early versions which do not include APNG support
themselves.

It is expected that this approach will remain compatible with later major
versions of [libpng](#libpng).

### LIBPNG {#libpng-use}

    #include <png.h> /* **DO NOT** include apng.h */

This only works if the version of libpng you are building against contains
native APNG support.  Once you have done this your application will only run
against libpng DLLs which are at least the version you built against.  Other,
earlier, versions will typically cause your application to crash immediately
it is executed.

## READING APNG CHUNKs {#core-read}

Reading an APNG is possible with any of the [libpng](#libpng) read APIs
**except** the "Simplified API".  The libpng Simplified API can and probably
should be used to read the standard PNG image but this is a separate step and
is only required if the APNG is to be displayed.

The following sequence uses the libpng _sequential_ reader.  Using the
_progressive_ reader allows incremental display of frames but requires a
non-linear code flow; decisions about what to do next have to be based on a
state machine.

1. `png_create_read_struct();`
  - Create a PNG read structure, then:

2. Set application unknown chunk handling if required.  This must be done
before the next step because it will update the unknown handling.

3. `apng_read_enable(png_ptr);`
  - This must be called to ensure that the chunk handling remains compatible
    with the APNG support.  Without this application unknown handling may cause
    APNG chunks to be skipped.

4. `png_read_info(png_ptr, pre_IDAT_info_ptr);`
  - This will populate `pre_IDAT_info_ptr` with the `acTL` chunk if present (if
    this is an APNG) and the fcTL chunk if one is present before the main image
    data (`IDAT`)

5. `apng_get_acTL(png_ptr, pre_IDAT_info_ptr, &num_frames, &num_plays);`
  - If this succeeds (returns true) decide whether to process the APNG based on
    the returned information.  If you do decide to handle the APNG you must call
    this:

6. `apng_get_fcTL(png_ptr, pre_IDAT_info_ptr, 0, {fcTL parameters});`
  - Returns true if an `fcTL` with sequence number 0 is stored in
    `pre_IDAT_info_ptr` and fills in the `fcTL` parameters.

  - Returns false if there is no `fcTL` before the first `IDAT` chunk; this
    means that the main PNG image is not part of the animation.

  - Determines the next sequence number; if `png_get_fcTL` returns `true` it
    is 1 else `fcTL[0]` will occur after the main image and the next sequence
    number is 0.

7. Read the main image; any of the APIs may be used.  It is also possible to
   set IDAT chunks to be handled as "unknown" to avoid decoding the main image
   at this point.

   [//]: #(TODO: does this work after `png_read_info`?)

8. `png_read_end(png_ptr, post_IDAT_info_ptr)`
  - This should be called with a separate `png_info_struct` otherwise the
    results of subsequent calls may be unable to determine error conditions or
    just be plain misleading.

9. **Animation loop** {#animation-loop}  to find all the frames in the
   animation:

  -  `apng_get_fcTL(png_ptr, post_IDAT_info_ptr, next_sequence_number++, &<fctl parameters>)`

    1. Returns true if an `fcTL` with `next_sequence_number` is stored in
       `post_IDAT_info_ptr` and fills in the `fcTL` parameters.

    2. Returns false at the end of the APNG frame list if the APNG is well
       formed.  Exit the [animation loop](#animation-loop); At this point the
       total count of frames successful calls to `png_get_fcTL`) will be
       equal to `num_frames` from `png_get_acTL` in a well formed APNG.

  - **Frame loop** {#frame-loop} to read all the `fdAT` chunks in the given
    frame:

    1. `apng_get_fdAT(png_ptr, post_IDAT_info_ptr, next_sequence_number, &num_bytes, &frame_data_ptr)`

      + Returns true if an `fdAT` with `next_sequence_number` is stored in
        `post_IDAT_info_ptr` and fills in num_bytes and `frame_data_ptr`
        appropriately.  `frame_data_ptr` is set to point to a cache of the
        next block of `IDAT` data; `png_const_byte frame_data[num_bytes]`.

        Increment `next_sequence_number` and continue the
        [frame loop](#frame-loop).

      + If false is returned either `next_sequence_number` is the `fcTL` of
        the next frame or the APNG chunk stream has ended or is malformed.
        Exit the [frame loop](#frame-loop) and call `png_get_fcTL` with the
        same sequence number to get the next `fcTL` or determine that the APNG
        has ended.

  Dropped chunks will cause the loops to fail early unless the error occurs
  with a dropped chunk after the initial `fdAT` chunks of the final frame.
  Truncation of the final frame cannot be determined without decompressing the
  sequence of chunks; this is a feature of the APNG design.

`apng_find_after(png_ptr, info_ptr, find_last, what)` is an optional assist
function that may be called with either `pre_IDAT_info_ptr` or
`post_IDAT_info_ptr` to determine whether the `sequence_number` list is
complete.  It returns the next or last sequence numbers in the supplied
`info_struct`.
