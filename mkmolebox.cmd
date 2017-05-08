
cd %~dp0
set CURR=%CD%
if exist xternal set XTERNAL=%~dp0xternal

for %%c in (Core MxbPack Molebox) do (
	cd %%c
	cmd /c makeit.cmd
	cd %CURR%
)

