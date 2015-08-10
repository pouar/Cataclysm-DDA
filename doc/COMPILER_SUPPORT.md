# Compilers Supported

We are compiling with -std=c++11 as you can see from the Makefile.
This regretably restricts which compilers will compile the project, while at the same time allowing the use of a plethora of features introduced by C++11.

## GCC

GCC is the preferred compiler for building on Linux.
Technically GCC doesn't "fully" support C++11 until version 4.8.1, but we aren't currently using any language features past 4.6 or so.  That is liable to change in the future.
At some point we are going to want to switch that back once enough people are using gcc 4.7, or some other killer feature comes along that makes us declare a hard dependency on 4.7.3+.

## Clang

We use Clang for our experimental and release builds.
Clang similarly fully supports C++11 as of version 3.1, but you may be able to get by with an earlier version.

## MinGW-w64 (MXE in either Cygwin or Linux)

This is the preferred compiler for building on Windows, and is the compiler we use to cross-compile for Windows experimental and release builds.
MinGW-w64 is currently building the project.  Input on the earliest version that will successfully compile the project is welcome.
