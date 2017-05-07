
TOPDIR  =..
INCLUDE = $(INCLUDE);$(TOPDIR)\include;$(TOPDIR)\Core\classes;$(TOPDIR)\Core\classes\RSA512.SRC
EXECUTABLE = $(PROJECT)


!if "$(TARGET_CPU)" != "X86"
all:
build:
rebuild:
info:
clean:
!else

!include $(XTERNAL)\Make.rules.mak

CCXFLAGS = $(CCXFLAGS) -D_TEGGOSTATIC -DSVFS_PACKER

SRCDIR  = src
OBJECTS = \
    $(OBJDIR)\resources.obj \
    $(OBJDIR)\rgsreg.obj \
    $(OBJDIR)\commandstream.obj \
    $(OBJDIR)\teggovbox.obj \
    $(OBJDIR)\iatgen.obj \
    $(OBJDIR)\xfile.obj \
    $(OBJDIR)\stubpack.obj \
    $(OBJDIR)\pwafx.obj \
    $(OBJDIR)\myafx.obj \
    $(OBJDIR)\fpattern_a.obj \
    $(OBJDIR)\version.obj \
    $(OBJDIR)\build_time.obj \
    $(OBJDIR)\main.obj \
    $(OBJDIR)\mxbpack_RC.obj \
    $(OBJDIR)\packager.obj \
    $(OBJDIR)\pebox.obj \
    $(OBJDIR)\splicer.obj \
    $(OBJDIR)\import.obj \
    $(OBJDIR)\rsa.obj \
    $(OBJDIR)\rsa_accel.obj \
    $(OBJDIR)\xorS.obj \
    $(OBJDIR)\classes.obj \
    $(OBJDIR)\newdes_accel.obj \
    $(OBJDIR)\cor0S.obj 

$(OBJDIR)\rsa.obj : $(TOPDIR)\Core\classes\RSA512.SRC\rsa.cpp
    $(CC) -c $(CCXFLAGS) $(DLL_OPTS) $(INCL) -Fo$@ $**

$(OBJDIR)\rsa_accel.obj : $(TOPDIR)\Core\classes\RSA512.SRC\rsa_accel.S
    $(AS) $(ASFLAGS) -o$@ $**

$(OBJDIR)\fpattern_a.obj : $(TOPDIR)\Core\src\fpattern_a.c
    $(CC) -c $(CCFLAGS) $(DLL_OPTS) $(INCL) -Fo$@ $**

$(OBJDIR)\version.obj : $(TOPDIR)\Core\src\version.c
    $(CC) -c $(CCFLAGS) $(DLL_OPTS) $(INCL) -Fo$@ $**

$(OBJDIR)\newdes_accel.obj: $(TOPDIR)\Core\classes\sources\newdes_accel.S
    $(AS) $(ASFLAGS) -o$@ $**

$(OBJDIR)\cor0S.obj : $(TOPDIR)\Core\cor0S.c
    $(CC) -c $(CCFLAGS) $(DLL_OPTS) $(INCL) -Fo$@ $**

LIBRARIES= advapi32.lib user32.lib shell32.lib ws2_32.lib mpr.lib zlib$(LIBSFX).lib libhash$(LIBSFX).lib

!if "$(STATIC_LIB)"!="YES"
!else
!endif

!include $(XTERNAL)\Make.exe.mak

!endif


