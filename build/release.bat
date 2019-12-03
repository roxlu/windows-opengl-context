@echo off
set d=%cd%
set bd=win-x86_64
set id=%cd%/../install/
set ed=%cd%/../extern/
set type="release"
set cmake_bt="Release"
set cmake_gen="Visual Studio 16 2019"
set compiler="msvc"
set debug_flag=""
set profile_enable="Off"
set profile_dir=


:: Iterate over command line arguments
:argsloop

  if "%1" == "" (
    goto :argsloopdone
  )

  if "%1" == "debug" (
    set type=debug
  )

  if "%1" == "profile" (
    set profile_enable=On
    set profile_dir=.profile
  )
  
  if "%1" == "clang" (
    set compiler="clang"
    set cmake_gen="Ninja"
  )
  
echo %1
shift
goto :argsloop
:argsloopdone


:: Create build dir name
set bd=%bd%%profile_dir%.%compiler%

if %type% == debug (
   set bd="%bd%d"
   set cmake_bt="Debug"
   set debug_flag="-debug"
)

if not exist %d%/%bd% (
   mkdir "%d%/%bd%"
)

if not exist %ed% (
   mkdir "%ed%"
)

cd %d%
cd %bd%

if %compiler% == "clang" (

  :: -w:  no warnings
  :: -EHsc: exception model
  
  cmake -G %cmake_gen% ^
        -DCMAKE_BUILD_TYPE="%cmake_bt%" ^
        -DCMAKE_INSTALL_PREFIX="%id%" ^
        -DCMAKE_CXX_COMPILER:PATH="C:\Program Files\LLVM\bin\clang-cl.exe" ^
        -DCMAKE_C_COMPILER:PATH="C:\Program Files\LLVM\bin\clang-cl.exe" ^
        -DCMAKE_LINKER:PATH="C:\Program Files\LLVM\bin\lld-link.exe" ^
        -DCMAKE_CXX_FLAGS="-D_CRT_SECURE_NO_WARNINGS -w -EHsc" ^
        -DENABLE_PROFILING=%profile_enable% ^
        ..

  if errorlevel 1 (
     echo "Failed to configure."
     goto:errorhandling
  )

  cmake --build . --target install --config %cmake_bt% --parallel 12 

  if errorlevel 1 (
    goto errorhandling
  )
 
) else (

  cmake -DCMAKE_BUILD_TYPE="%cmake_bt%" ^
        -DCMAKE_INSTALL_PREFIX="%id%" ^
        -DPOLY_DIR=./../extern/roxlu-polytrope/ ^
        -DENABLE_PROFILING=%profile_enable% ^
        -G %cmake_gen% ^
        -A X64 ^
        ..

  if errorlevel 1 (
    goto errorhandling
  )    

  cmake --build . ^
        --target install ^
        --config %cmake_bt% ^
        --parallel 12 -- /nologo /verbosity:normal /nowarn:4244 /clp:NoItemAndPropertyList

  if errorlevel 1 (
    goto errorhandling
  )
)

:: Run
cd %id%/bin
:: test-create-gl-context.exe
:: test-basic-window.exe
:: test-shared-context
test-shared-context-threading
echo "Executed"
cd %d%
goto success

:: Error or success handlers.
:errorhandling
echo "Error occured."
cd %d%      
goto:eof

:success
echo "Ready"
cd %d%
goto:eof
