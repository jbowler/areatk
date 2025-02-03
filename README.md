The APNG reading, editing and authoring toolkit.
================================================

The APNG reading, editing and authoring toolkit, **AREATK**, is a toolkit which uses the official libpng to read, edit and author Animated PNG, *APNG*, files.

libapng
=======

The core of AREATK is a library called **libapng**.  This library provides extensions to the standard PNG library, **libpng**, that support reading and writing the additional PNG "chunks" designed to support animation.

The library relies on the API of libpng 1.6 but does not require a specific version of libpng.
