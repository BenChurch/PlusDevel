@ECHO off 
ECHO Build PlusLib ApiReference...

rem IS_MAKE_PROGRAM_MSBUILD?
if TRUE==TRUE goto MakeUsingMsBuild

rem ---------------- msdev -----------------------
:msdev

ECHO   Clear old files to force rebuild of documentation
"C:/Program Files (x86)/MSBuild/12.0/bin/MSBuild.exe" PlusLib.sln /clean Release /project Documentation-PlusLib-ApiReference 1> CreateDoc.log 2>&1 
IF ERRORLEVEL 1 GOTO fail

ECHO   Generating Documentation-PlusLib-ApiReference
"C:/Program Files (x86)/MSBuild/12.0/bin/MSBuild.exe" PlusLib.sln /build Release /project Documentation-PlusLib-ApiReference 1>> CreateDoc.log 2>&1 
IF ERRORLEVEL 1 GOTO fail

goto success

rem ---------------- msbuild -----------------------
:MakeUsingMsBuild

ECHO   Clear old files to force rebuild of documentation
"C:/Program Files (x86)/MSBuild/12.0/bin/MSBuild.exe" src/Documentation/ApiReference/Documentation-PlusLib-ApiReference.vcxproj /p:Configuration=Release /target:clean 1> CreateDoc.log 2>&1 
IF ERRORLEVEL 1 GOTO fail

ECHO   Generating documentation
"C:/Program Files (x86)/MSBuild/12.0/bin/MSBuild.exe" src/Documentation/ApiReference/Documentation-PlusLib-ApiReference.vcxproj /p:Configuration=Release /target:rebuild 1>> CreateDoc.log 2>&1
IF ERRORLEVEL 1 GOTO fail

goto success

rem ---------------------------------------

:success
ECHO Documentation available at: C:/D/PBE/bin/Doc
goto end

:fail
ECHO Failed to generate documentation
goto end

:end
