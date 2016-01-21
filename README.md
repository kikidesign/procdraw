Procdraw
========

Procdraw is/will be a platform for exploring programming language
design and interaction design for live programming.

In its current form, it consists of:

* An interpreter for a Lisp-like language with:
    * s-expression syntax
    * list-structured memory
    * procedures stored as list structures and always recoverable and
      modifiable at run time
* A data-flow model ("signals")
* The beginnings of a 3D graphics API using OpenGL

The primary target platform is currently Linux but, where possible,
Procdraw uses cross-platform technologies (such as CMake, SDL2, and
OpenGL) with the hope of easing the work of porting to other platforms
in the future.

Building on Linux
-----------------

Dependencies:

* CMake
* GNU C++ compiler
* GNU Readline
* SDL2
* OpenGL
* GLEW
* GLM
* FreeType 2
* ALSA (for MIDI input)
* Libxml2
* xsltproc (for building the Documentation)

Procdraw uses the CMake build system. To build Procdraw and run the
tests:

```
$ mkdir build
$ cd build
$ cmake ..
$ make
$ ctest --output-on-failure
```

To make a debug build, invoke CMake with the `-DCMAKE_BUILD_TYPE=Debug` option:

```
$ cmake -DCMAKE_BUILD_TYPE=Debug ..
```

To generate the documentation, run `make html` in the `build` directory:

```
$ make html
```

Building on Windows
-------------------

Dependencies:

- Visual Studio Community 2015 (with C++ tools and Windows SDK)
- CMake

Run CMake to generate the Visual Studio solution:

```
> mkdir build
> cd build
> cmake -G "Visual Studio 14" ..
```

To build in Visual Studio:

- Open `build\Procdraw.sln`
- Build Solution (Ctrl+Shift+B)

To run the tests in Visual Studio:

- Build the `RUN_TESTS` project

Running Procdraw from Emacs
---------------------------

Start by configuring Procdraw as the inferior lisp program:

```
(setq inferior-lisp-program "/path/to/procdraw")
```

Procdraw can then be started from Emacs with:

```
M-x run-lisp
```

When expressions are evaluated (C-x C-e) in Lisp mode, they will be
sent to Procdraw. To make a Lisp buffer, you can open a Lisp file, or
create a new buffer with:

```
C-x b BUFFER-NAME (such as *procdraw*)
M-x lisp-mode
```

Connecting ALSA MIDI to Procdraw
--------------------------------

```
$ aconnect -l
client 0: 'System' [type=kernel]
    0 'Timer           '
    1 'Announce        '
client 14: 'Midi Through' [type=kernel]
    0 'Midi Through Port-0'
client 24: 'Alias_8' [type=kernel]
    0 'Alias_8 MIDI 1  '
    1 'Alias_8 MIDI 2  '
client 128: 'Procdraw' [type=user]
    0 'Procdraw Input  '

$ aconnect 24:0 128:0
```
