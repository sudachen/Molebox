
@echo off

set CPU=X86
set BUILD=RELEASE
set goal=


set PYTHON_HOME=c:\Python27
set PATH=%XTERNAL%\Env;%PYTHON_HOME%;%PATH%
cd %~dp0
set ENV=env32_100.cmd
set BUILDBOT_VERSION_INCREMENT=YES

:repeat
if "%1" == "debug" set BUILD=DEBUG
if "%1" == "release" set BUILD=RELEASE
if "%1" == "clean" set goal=%goal% clean
if "%1" == "all" set goal=%goal% all
if "%1" == "build" set goal=%goal% all
if "%1" == "rebuild" set goal=clean all
if "%1" == "-t" goto :add_goal
shift
if "%1" neq "" goto :repeat

%ENV% nmake -f Makefile.mak %goal%

goto :eof

:add_goal
	shift 
	set goal=%goal% %1
	shift 
goto :repeat
