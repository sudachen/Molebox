
set MSRT=_100
set XTERNAL_TARGET=x86
set LIBRARIES=zlib libtiff libpng libjpeg libwx3 expatw libhash
set LIBRARIES2=zlib libhash libpict

cd %~dp0
set CURR=%CD%
if exist "%~dp0xternal" set XTERNAL=%~dp0xternal
cd "%XTERNAL%"
for %%i in (%LIBRARIES%) do cmd /c make_one.cmd %%i dll x86 rt100
for %%i in (%LIBRARIES2%) do cmd /c make_one.cmd %%i static x86 rt100
cd %CURR%

for %%c in (~Release ~Debug) do (
	if not exist %%c mkdir %%c
	for %%i in (%LIBRARIES%) do copy "%XTERNAL%\%%c"\%%i_32r10*.dll %%c
)

