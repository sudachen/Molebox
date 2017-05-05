
.SUFFIXES: .S .c .cpp .obj
TOPDIR=..\..\..

!if "$(TEMP_BUILD)"==""
TEMP_BUILD=$(TEMP)
!endif

TMPDIR=$(TEMP_BUILD)\mkfw_WORKSPACE
OBJDIR=$(TMPDIR)\obj
BINDIR=..\..
XTERNAL=$(TOPDIR)\xternal
PATH=$(XTERNAL)\src\_Env;$(XTERNAL)\src\_Env\gcc\i386-mingw32\bin;$(PATH)

TARGETS = $(BINDIR)\loader.lso $(BINDIR)\mkfw.exe
all: $(TMPDIR) $(OBJDIR) $(TARGETS)

$(TMPDIR) $(OBJDIR):
	md $@

$(BINDIR)\loader.lso: $(OBJDIR)\fwinstall.obj $(OBJDIR)\fwaux.obj $(OBJDIR)\fwndes.obj $(OBJDIR)\fwstart.obj $(OBJDIR)\fwxor.obj
	gcc $** --shared -Wl,-T,fwlds.x -Wl,-nostdlib -Wl,-s -Wl,-Map -Wl,$@.txt -o $@ --no-exceptions        
$(BINDIR)\mkfw.exe: $(OBJDIR)\mkfw.obj $(OBJDIR)\lzss.obj $(BINDIR)\loader.lso
	vs2010 link /debug /out:$@ /pdb:$@.pdb /incremental:no $(OBJDIR)\mkfw.obj $(OBJDIR)\lzss.obj libpe-mt10d.lib libhash-mt10d.lib
	
{}.c{$(OBJDIR)}.obj:
	gcc -c -o $@ --no-exceptions -O1 -pie -I../../include $<
{}.S{$(OBJDIR)}.obj:
	as -o $@ $<

$(OBJDIR)\mkfw.obj : mkfw.cpp $(XTERNAL)\src\libpe\include\libpe.h
	vs2010 cl /c /FC /MTd -Fo$@ /Zi /EHsc mkfw.cpp -I../../include
$(OBJDIR)\lzss.obj : lzss.c
	vs2010 cl /c /FC /MTd -Fo$@ /Zi lzss.c

clean:
	-del /q/s  $(OBJDIR)\fwinstall.obj $(OBJDIR)\fwaux.obj $(OBJDIR)\fwndes.obj $(OBJDIR)\fwstart.obj $(OBJDIR)\fwxor.obj
	-del /q/s  $(OBJDIR)\mkfw.obj $(OBJDIR)\lzss.obj
	-del /q/s  $(TARGETS)
