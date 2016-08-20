@ECHO off 

SET BUILD_TYPE=Release

REM Skip build step if continuous or individual mode

REM Skip clean build step if not nightly mode
if "%1" == "-C" goto cleansuccess
if "%1" == "-I" goto cleansuccess
if "%1" == "" goto cleansuccess
if "%1" == "-E" goto cleansuccess

ECHO Clean...
rem IS_MAKE_PROGRAM_MSBUILD?
if TRUE==TRUE goto CleanUsingMsBuild
rem ---------------- msdev clean -----------------------
"C:/Program Files (x86)/MSBuild/12.0/bin/MSBuild.exe" PlusLib.sln /clean %BUILD_TYPE% /project ALL_BUILD
IF ERRORLEVEL 1 GOTO buildfail
goto cleansuccess
rem ---------------- msbuild build -----------------------
:CleanUsingMsBuild
"C:/Program Files (x86)/MSBuild/12.0/bin/MSBuild.exe" ALL_BUILD.vcxproj /p:Configuration=Release /target:clean
IF ERRORLEVEL 1 GOTO buildfail
goto cleansuccess

:cleansuccess

rem ---------------------------------------
if "%1" == "" goto experimental
if "%1" == "-E" goto experimental
if "%1" == "-N" goto nightly
if "%1" == "-C" goto continuous
if "%1" == "-I" goto individual

:experimental
"C:/Program Files/CMake/bin/ctest.exe" -C %BUILD_TYPE% -D Experimental --output-on-failure
goto success

:nightly
@REM Clean before the nightly build to enforce all build warnings appear on all nightly dashboard submissions
"C:/Program Files/CMake/bin/ctest.exe" -C %BUILD_TYPE% -D Nightly
goto success

:continuous
"C:/Program Files/CMake/bin/ctest.exe" -C %BUILD_TYPE% -D Continuous
@REM Wait for some time before continue to allow checking the results of the executions
timeout /t 15
goto success

:individual 
@REM Run individual tests with regexp search 
@REM Display the list of tests
"C:/Program Files/CMake/bin/ctest.exe" -C %BUILD_TYPE% -R "%2" -N
@REM Run selected tests
"C:/Program Files/CMake/bin/ctest.exe" -C %BUILD_TYPE% -R "%2" -V
goto success

:success
exit /b 0

:buildfail
ECHO Failed to build PlusLib
exit /b 1
