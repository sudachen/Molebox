
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

CCXFLAGS = $(CCXFLAGS) -D_TEGGOSTATIC -DUNICODE -D_GUI_APP_

SRCDIR  = src
OBJECTS = \
    $(OBJDIR)\classes.obj \
    $(OBJDIR)\renamemove.obj \
    $(OBJDIR)\queryaddfiles.obj \
    $(OBJDIR)\newsubfolder.obj \
    $(OBJDIR)\messages.obj \
    $(OBJDIR)\processdlg.obj \
    $(OBJDIR)\config.obj \
    $(OBJDIR)\about.obj \
    $(OBJDIR)\mxf.obj \
    $(OBJDIR)\mainframe.obj \
    $(OBJDIR)\myafx.obj \
    $(OBJDIR)\version.obj \
    $(OBJDIR)\build_time.obj \
    $(OBJDIR)\newdes_accel.obj \
    $(OBJDIR)\molebox_RC.obj

$(OBJDIR)\version.obj : $(TOPDIR)\Core\src\version.c
    $(CC) -c $(CCFLAGS) $(DLL_OPTS) $(INCL) -Fo$@ $**

$(OBJDIR)\newdes_accel.obj: $(TOPDIR)\Core\classes\sources\newdes_accel.S
    $(AS) $(ASFLAGS) -o$@ $**

LIBRARIES= oleaut32.lib advapi32.lib user32.lib shell32.lib ws2_32.lib mpr.lib zlib$(LIBSFX).lib libwx3$(LIBSFX).lib libtiff$(LIBSFX).lib libjpeg$(LIBSFX).lib libpng$(LIBSFX).lib expatw$(LIBSFX).lib libhash$(LIBSFX).lib

!if "$(STATIC_LIB)"!="YES"
!else
!endif

!include $(XTERNAL)\Make.exe.mak

!endif


