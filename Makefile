#
# Karlik - Programming for preschoolers
#
# Copyright 2020 Jiri Svoboda
#
# Permission is hereby granted, free of charge, to any person obtaining 
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#

CC	= gcc
CFLAGS	= -Wall -Werror `pkg-config --cflags sdl2` -ggdb -Og
LIBS	= `pkg-config --libs sdl2`

sources = \
	gfx.c \
	main.c \
	map.c \
	toolbar.c

headers = $(wildcard types/*.h *.h)
objects = $(sources:.c=.o)

output	= karlik

all: $(output)

$(output): $(objects)
	$(CC) $(LIBS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

ccheck:
	ccheck-run.sh $(PWD)

clean:
	rm -f $(output) $(objects)