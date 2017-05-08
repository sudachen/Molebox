PROJECT=Core
BASEDIR=..
SRCDIR=.
!include $(BASEDIR)\Makefile.rules

TARGETS=$(LIBDIR)\Core$(DBGSFX).lib $(BINDIR)\cor1$(DLLSFX).dll
_all: $(OBJDIR) $(LIBDIR) $(BINDIR) $(TMPDIR) $(TARGETS)

OBJECTS= \
    $(OBJDIR)\xorS.obj \
    $(OBJDIR)\xorC.obj \
    $(OBJDIR)\apif_register.obj \
    $(OBJDIR)\apif_sectionobjects.obj \
    $(OBJDIR)\apif_fileobjects.obj \
    $(OBJDIR)\apif_actctx.obj \
    $(OBJDIR)\apif_crproc.obj \
    $(OBJDIR)\apif_cmdline.obj \
    $(OBJDIR)\apif.obj \
    $(OBJDIR)\vfs.obj \
    $(OBJDIR)\svfs.obj \
    $(OBJDIR)\regimport.obj \
    $(OBJDIR)\fpattern_a.obj \
    $(OBJDIR)\fpattern_w.obj \
    $(OBJDIR)\import.obj \
    $(OBJDIR)\logger.obj \
    $(OBJDIR)\myaux.obj \
    $(OBJDIR)\splicer.obj \
    $(OBJDIR)\classes.obj \
    $(OBJDIR)\version.obj \
    $(OBJDIR)\public_api.obj \
    $(OBJDIR)\argv.obj \
    $(OBJDIR)\splash_pp.obj \
    $(OBJDIR)\rsa.obj \
    $(OBJDIR)\rsa_accel.obj \
    $(OBJDIR)\selfload.obj \
    $(OBJDIR)\auxS.obj \
    $(OBJDIR)\apifS.obj \
    $(OBJDIR)\splash.obj \

$(OBJDIR)\xorS.obj : $(SRCDIR)\xorS.S 
	@echo $(*F).S
	@$(AS) $(_M64) -o$@ $(SRCDIR)\xorS.S
$(SRCDIR)\xorS.S: 
	..\XorS.py $(SRCDIR)\xorS.S

$(OBJDIR)\xorC.obj : $(SRCDIR)\xorC.S
	@echo $(*F).S
	@$(AS) $(_M64) -o$@ $(SRCDIR)\xorC.S
$(SRCDIR)\xorC.S: 
	..\XorC.py $(SRCDIR)\xorC.S

{$(BASEDIR)\classes\sources\Z}.c{$(OBJDIR)}.obj:
	@cl $(CFLAGS) $(INCL) -c -Fo$@ $<

{$(BASEDIR)\classes\RSA512.SRC}.cpp{$(OBJDIR)}.obj:
	@cl $(CCFLAGS) $(INCL) -c -Fo$@ $<

{$(BASEDIR)\classes\RSA512.SRC}.S{$(OBJDIR)}.obj:
	@echo $(*F)
	@$(AS) $(_M64) -o$@ $<

{$(BASEDIR)\..}.c{$(OBJDIR)}.obj:
	@cl $(CFLAGS) $(INCL) -c -Fo$@ $<

$(LIBDIR)\Core$(DBGSFX).lib: $(OBJECTS)
	@echo -- collect -- $@
	@link /lib -nologo -machine:$(CPU) -out:$@ $**

$(OBJDIR)\DLL.res: $(BASEDIR)\DLL.rc
    @echo DLL.rc
    @rc /fo $@ $**

MERGE = \
 	-align:0x4000 \
	-section:.text,RWE \
	-merge:.iX=.text \
	-merge:.A0=.text \
	-merge:.A1=.text \
	-merge:.A2=.text \
	-merge:.A3=.text \
	-merge:.A4=.text \
	-merge:.A5=.text \
	-merge:.A6=.text \
	-merge:.A7=.text \
	-merge:.A8=.text \
	-merge:.A9=.text \
	-merge:.data=.text \
	-merge:.bss=.text \
	-merge:.k=.text \
	-merge:.xdata=.text \
	-merge:.xbss=.text \
	-merge:.z=.text \
	-merge:.XORS=.text \
	-merge:.rdata=.text \
	-merge:.edata=.expo \

$(BINDIR)\cor1$(DLLSFX).dll: $(LIBDIR)\Core$(DBGSFX).lib $(OBJDIR)\DLL.res $(LIBDIR)\Xi.lib 
	@echo -- collect -- $@
	@link /dll /pdb:$(BINDIR)\cor1$(DLLSFX).pdb /map:$(BINDIR)\cor1$(DLLSFX).map.txt /debug /nologo /machine:$(CPU) /out:$@ \
			$** libhash$(LIBSFX).lib zlib$(LIBSFX).lib libpict$(LIBSFX).lib \
			/def:$(BASEDIR)\DLL.def /incremental:no /noentry /opt:icf /opt:ref $(MERGE)

clean:
	-del /q /f /s $(OBJECTS) 
	-del /q /f /s $(TARGETS)
	-del /q /f /s $(OBJDIR)\XorS.S
	-del /q /f /s $(OBJDIR)\XorC.S
