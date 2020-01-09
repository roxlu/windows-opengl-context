# Creating Shared OpenGL Contexts 

I created this repository while looking into an [issue with
Filament](https://github.com/google/filament/issues/1921). This
repository contains 3 tests:

- _test-research.cpp_: Contains a summary of how to create a
  OpenGL context on Windows and some background info.
  
- _test-shared-context.cpp_: Contains code that creates a couple
  of OpenGL contexts using the recommended flow (e.g. using a
  temporary context).

- _test-shared-context-threading.cpp_: Similar to
  _test-shared-context.cpp_ but here, we simulate how Filament
  creates a shared context, most importantly that we create the
  context in a separate thread.

## Building with Clang

- Make sure that you have [Clang](http://releases.llvm.org/download.html) installed into `C:/Program Files/LLVM`
- Make sure that you have [CMake 3.15+](https://cmake.org/download/) installed
- Make sure you have [Ninja](https://ninja-build.org/) installed.
- Open a *x64 Native Tools Command Prompt for VS 2019*
- Go into the `build` directory
- Run `release.bat clang` 

## Building with MSVC

- Make sure that you have [Clang](http://releases.llvm.org/download.html) installed into `C:/Program Files/LLVM`
- Make sure that you have [CMake 3.15+](https://cmake.org/download/) installed
- Make sure you have [Ninja](https://ninja-build.org/) installed.
- Open a *x64 Native Tools Command Prompt for VS 2019*
- Go into the `build` directory
- Run `release.bat`

## Solution (?)

[baldurk](https://www.twitter.com/baldurk) pointed me to
something which seems to be the cause of the issue. I'm creating a
new OpenGl context in a separate thread and try to use the OpenGl
context from the main as a shared context. Though the OpenGl
context from the main thread is still *current*.  When I unset
the current OpenGl context from the main thread things seem to
work! [See this, where I unset the current context](https://github.com/roxlu/windows-opengl-context/blob/master/src/test-shared-context-threading.cpp#L62-L71).

**IMPORTANT**: when you create an OpenGl context and you want to
share this context with a context that you create in another
thread, make sure that the context is **not** current.
