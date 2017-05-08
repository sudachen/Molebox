
if exist "%~dp0xternal" set XTERNAL="%~dp0xternal"
echo %XTERNAL%

cd %~dp0

set DIST=Molebox_GPL
if not exist %DIST% mkdir %DIST%

del /q %DIST%\*
del /q molebox_gpl.zip

set LIBRARIES=zlib libwx3 libtiff libpng libjpeg expatw libhash

copy ~Release\molebox.exe %DIST%
copy ~Release\mxbpack.exe %DIST%
for %%i in (%LIBRARIES%) do copy ~Release\%%i_32r10.dll %DIST%

%XTERNAL%\Env\7z a -tzip molebox_gpl.zip %DIST%

del /q %DIST%\*
rmdir %DIST%


