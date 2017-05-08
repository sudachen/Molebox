
cd %~dp0
set CURR=%CD%
if exist "%~dp0xternal" set XTERNAL="%~dp0xternal"
echo %XTERNAL%

for %%c in (Core MxbPack Molebox) do (
	cd %%c
	cmd /c makeit.cmd
	cd %CURR%
)

