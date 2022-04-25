Karlík - Programming for preschoolers
=====================================
Copyright 2022 Jiří Svoboda

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
Karlík is still under construction and not functionally complete.

TODO
----
  * Proper images for error dialogs
  * Icon dialog to provide icon for new procedure
    * List of predrawn and existing icons
    * Magnified view / drawing canvas
    * Palette
    * Preview
    * Accept button
  * Delete procedure
  * Animate program execution step by step
    * Show procedure / statement currently being executed in program view
  * Calling procedure from procedure (call stack)
  * Recursion
  * Conditional statement
  * Loop statement

Compiling
---------

You need a working GNU toolchain (GCC, Binutils, Make) (Linux or similar OS)
and development files for SDL2, the multimedia library.

To build the software simply type:

    $ make

To start Karlík, just type:

    # ./karlik

Using Karlík
------------
Karlík is very simple to understand and control. It does not use any text,
only images. It is controlled using a mouse (or other pointing device)
with just one button, using just clicks (no drag and drop, no doubleclick),
making it suitable for children and beginners.

Karlík automatically saves its state upon exit and restores it upon start.
You can exit Karlík any time by pressing Escape or closing its window.
When it is started next time, Karlík will continue exactly where it stopped,
everything will be preserved.

When you start Karlík up, on the top of the screen you can see the main
toolbar. It allows you to select between two main modes: Vocabulary and Map.
(Karlík starts up in Map mode by default).

Map mode allows you to modify the city's map. The second toolbar allows you
to select what you would like to place in the square: Wall, White tag,
Grey tag, Black tag, Robot or Delete. Then click on the map to place
or delete an object. The delete tool deletes all objects on a square.
(A square can contain a robot and a tag, but not multiple tags, nor
wall and something else).

Vocabulary mode allows you to give the robot orders, to teach it new commands
and examine existing commands. The second toolbar contains the Work,
Learn and Examine icons.

With the Work icon selected, you can give the robot commands. You do this
by clicking on one of the verb icons at the bottom of the screen. If
the robot cannot fulfill the command, an error dialog will be popped up
containing a graphical representation of the specific error condition.
There are three possible errors: (1) Robot hit a wall, (2) Robot tried
to put down a tag on an already occupied square, (3) Robot tried to pick
up a tag from an empty square. Clicking the error dialog will dismiss it.

With the Learn icon selected, you can teach the robot a new command.
Any new command consists of one or more pre-existing commands. You can
append a new command by clicking one of the verb icons at the bottom of
the screen. The robot does not executed the command, it just remembers it.
The commands being entered are displayed on the right-hand side of the screen.

Once done, click the Bell verb to finish entering the commands.

With the Examine icon selected, you can examine existing complex commands.
To examine a command, click on the corresponding verb icon at the bottom
of the screen. The details of the command are now displayed on the
right-hand side of the screen.

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

This requires the `ccheck` tool from the [Sycek project][1]. Ccheck should
not report any warnings.

To run Clang analyzer type:

    $ make clean && scan-build make

The analyzer should produce no warnings or errors.

[1]: https://github.com/jxsvoboda/sycek
