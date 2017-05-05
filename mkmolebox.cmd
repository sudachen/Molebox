
cd %~dp0
set CURR=%CD%

for %%c in (Core MxbPack Molebox) do (
	cd %%c
	cmd /c makeit.cmd
	cd %CURR%
)

