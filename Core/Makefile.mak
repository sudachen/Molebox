
all: Xi_Build Core_Build Cor0S
clean: Xi_Clean Core_Clean

Xi_Build: 
	@echo build Xi.lib
	@cd Xi
	@$(MAKE) /nologo -f Makefile.mak
	@cd ..

Xi_Clean: 
	@echo clean Xi.lib
	@cd Xi
	@$(MAKE) /nologo -f Makefile.mak clean
	@cd ..

Core_Build:
	@echo build cor1xxx.dll
	@cd src
	@$(MAKE) /nologo -f Makefile.mak
	@cd ..

Core_Clean:
	@echo clean core.lib
	@cd src
	@$(MAKE) /nologo -f Makefile.mak clean
	@cd ..

molebox.BIN.C molebox.BIN: build_molebox_BIN
build_molebox_BIN: mkfw.exe loader.lso
	mkfw.exe 

_lzss.pyd:
    	cl -nologo -Z7 -LD -MT -Ox _lzss.c classes/sources/lz77ss.cpp -I"$(PYTHON_HOME)/include" "$(PYTHON_HOME)/Libs/python27.lib" -Fe"_lzss.pyd" -link -dll -debug -incremental:no -export:init_lzss 

Cor0S: Core_Build _lzss.pyd
!if "$(BUILD)" == "RELEASE"
	$(PYTHON_HOME)\python ecore.py ../~Release/cor1rel.dll ../~Release/cor1rel.Biz ../~Release/cor1rel.raw
        mbin2c ../~Release/cor1rel.Biz core0S > cor0S.c
!endif	

	