Karlík - Programming for preschoolers
=====================================
Copyright 2020 Jiří Svoboda

Permission is hereby granted, free of charge, to any person obtaining 
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

Introduction
------------
Karlík is Karel's little brother. He can't read or write very well yet,
but he'd love to learn some structured programming. Karlík's vocabulary
is based on pictures, not words, making it accessible to anybody of any
age and speaking any language.

Karlík lives in a city built on a square grid. Each tile contains either
nothing, a wall, a tag (white, grey or black), and possibly a robot.
At any time Karlík is standing on one square, facing one of the cardinal
directions (Nort, South, East, West). He's carrying a backpack that holds
a potentially infinite supply of white, grey and black tags.

When Karlík was born he only knew how to do four basic actions

  * step forward
  * turn left
  * put down a tag of a specific color
  * pick up a tag

Karlík is eager to learn more. Can you help him?

WARNING
-------
Karlík is in early stages of development and functionality is minimal.

Compiling
---------

You need a working GNU toolchain (GCC, Binutils, Make) (Linux or similar OS)
and development files for SDL2, the multimedia library.

To build the software simply type:

    $ make

To start Karlík, just type:

    # ./karlik

Acknowledgements
----------------
I'd like to thank Richard E. Pattis for creating our beloved friend
Karel the Robot.

I'd like to give a nod to the best book on programming ever written anywhere
in the multiverse

  * Markéta Synovcová: Martina si hraje spočítačem, 107 programů pro robota
    Karla, 1989 (Martina plays with the computer, 107 programs for Karel the
    Robot)

Maintainance notes
------------------
To check ccstyle type

    $ make ccheck

This requires the `ccheck` tool from the [Sycek project][1]

[1]: https://github.com/jxsvoboda/sycek
