@echo off
set d=%cd%
set bd=win-x86_64
set id=%cd%/../install/
set ed=%cd%/../extern/
set type="release"
set cmake_bt="Release"
set cmake_gen="Visual Studio 16 2019"
set debug_flag=""
set profile_enable="Off"
set profile_dir=

:: Iterate over command line arguments
:argsloop 
if "%1" == "" goto :argsloopdone
if "%1" == "debug" ( set type=debug )
if "%1" == "profile" (
   set profile_enable=On
   set profile_dir=.profile
)
echo %1
shift
goto :argsloop
:argsloopdone

:: Create build dir name
set bd=%bd%%profile_dir%

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

:: Configure
cmake -DCMAKE_BUILD_TYPE=%cmake_bt% ^
      -DCMAKE_INSTALL_PREFIX="%id%" ^
      -G %cmake_gen% ^
      -A X64 ^
      ..

if errorlevel 1 goto errorhandling

:: Build
cmake --build . ^
      --target install ^
      --config %cmake_bt% -- /nologo /verbosity:normal /nowarn:4244 /clp:NoItemAndPropertyList

if errorlevel 1 goto errorhandling

:: Run
cd %id%/bin
:: test-create-gl-context.exe
test-basic-window.exe
echo "Executed"
goto success

:: Error or success handlers.
:errorhandling
echo "Error occured."
exit

:success
echo "Ready"
