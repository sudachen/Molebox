@echo off 

set MAKEIT_DIR=%~dp0
set MAKEIT_REL=release
set WORKSPACE=MOLEBOX
set MSRT=_100
set ENVIRONS=env32

cmd /c %XTERNAL%\makeit_proc.cmd x86 dll %*
