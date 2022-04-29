#
# Karlik - Programming for preschoolers
#
# Copyright 2022 Jiri Svoboda
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
	adt/list.c \
	canvas.c \
	dir.c \
	errordlg.c \
	gfx.c \
	icondict.c \
	icondlg.c \
	karlik.c \
	main.c \
	map.c \
	mapedit.c \
	mapview.c \
	prog.c \
	progview.c \
	robot.c \
	robots.c \
	toolbar.c \
	vocabed.c \
	wordlist.c

headers = $(wildcard *.h)
objects = $(sources:.c=.o)

output	= karlik
launcher = Karlik.desktop

all: $(output) $(launcher)

$(output): $(objects)
	$(CC) $(LIBS) -o $@ $^

%.o: %.c $(headers)
	$(CC) $(CFLAGS) -c -o $@ $<

$(launcher):
	./mklauncher.sh $(PWD) >$@
	chmod 755 $@

ccheck:
	ccheck-run.sh $(PWD)

clean:
	rm -f $(output) $(objects) $(launcher)
