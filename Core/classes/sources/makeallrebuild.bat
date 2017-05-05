for /F %%i in ('cd') do set P=%%i
set PATH=%P%\..\..\Python26;%P%\..\..\C++\mingw32;%PATH%

set PSDK=%P%\..\..\C++\PSDK
set PSDK_INCLUDE=%PSDK%\include
set PSDK_LIB=%PSDK%\lib
set CLDIR=%P%\..\..\C++\7
set PATH=%CLDIR%\bin;%PSDK%\bin;%PATH%
set LIB=%PSDK%\lib;%CLDIR%\lib
set INCLUDE=%PSDK%\include;%CLDIR%\include


python ./makeit.py shared debug clean
python ./makeit.py shared clean
python ./makeit.py debug clean
python ./makeit.py clean
python ./makeit.py debug
python ./makeit.py
