
cd %~dp0
set CURR=%CD%
if exist xternal set XTERNAL=%~dp0xternal
cd %XTERNAL%
cmd /c build_all_vc10.cmd
cd %CURR%
set LIBRARIES=zlib libwx3 libtiff libpng libjpeg expatw libhash

for %%c in (~Release ~Debug) do (
	if not exist %%c mkdir %%c
	for %%i in (%LIBRARIES%) do copy %XTERNAL%\%%c\%%i_32r10*.dll %%c
)

