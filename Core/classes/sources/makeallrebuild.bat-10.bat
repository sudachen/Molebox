for /F %%i in ('cd') do set P=%%i
set PATH=%P%\..\..\Python26;%P%\..\..\C++\mingw32;%PATH%

set PSDK=%P%\..\..\C++\PSDK71
set PSDK_INCLUDE=%PSDK%\include
set PSDK_LIB=%PSDK%\lib
set CLDIR=%P%\..\..\C++\10.1
set PATH=%CLDIR%\bin;%PSDK%\bin;%PATH%
set LIB=%PSDK%\lib;%CLDIR%\lib
set INCLUDE=%PSDK%\include;%CLDIR%\include


python ./makeit.py -j vc10 debug clean
python ./makeit.py -j vc10 clean
python ./makeit.py -j vc10 debug
python ./makeit.py -j vc10 
