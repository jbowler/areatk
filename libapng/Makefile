#!/usr/bin/make -f
#
# This file is part of the APNG reading, editing and authoring toolkit.
#
# Copyright (c) 2025 John Bowler
#
# AREATK is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License, or (at your option) any later
# version.
#
# AREATK is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# AREATK. If not, see <https://www.gnu.org/licenses/>.

all: library test tools

library::
test::
tools::

# Override these from the command line with CFLAGS_OVERRIDE if necessary, e.g.
# if your compiler does not support C23
CFLAGS=-Wall -Wextra -Werror\
       -Wundef -Wwrite-strings -Wpointer-arith -Wmissing-declarations\
       -Wstrict-prototypes -Wmissing-prototypes -Wno-implicit-fallthrough\
       -g -O2 --std=c23 -pedantic $(CFLAGS_OVERRIDE)

CPPFLAGS=-DAPNGAPI=

CORE_FILES=core/add_fcTL.o core/add_fdAT.o core/find_after.o core/get_acTL.o\
	core/get_fcTL.o core/get_fdAT.o core/read_enable.o core/search.o\
	core/set_acTL.o

ADVANCED_FILES=

LIBAPNG_FILES=$(CORE_FILES) $(ADVANCED_FILES)

clean::
	$(RM) $(LIBAPNG_FILES)

libapng.a: $(LIBAPNG_FILES)
	$(AR) $(ARFLAGS) $@ $?

library:: libapng.a

clean::
	$(RM) libapng.a

test-core-read: test/core_read.o libapng.a
	$(CC) $(CFLAGS) -o $@ $^ -lpng -lz

test:: test-core-read

clean::
	$(RM) test-core-read test/core_read.o

$(LIBAPNG_FILES): inc/apng_private.h inc/apng.h

test/core_read.o: inc/apng.h
