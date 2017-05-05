PROJECT=XI
BASEDIR=..
SRCDIR=.
!include $(BASEDIR)\Makefile.rules

TARGET = $(LIBDIR)\Xi.lib

OBJECTS = \
		$(OBJDIR)\kernel32.obj \
		$(OBJDIR)\gdi32.obj \
		$(OBJDIR)\advapi32.obj \
		$(OBJDIR)\ntdll.obj \
		$(OBJDIR)\ole32.obj \
		$(OBJDIR)\oleaut32.obj \
		$(OBJDIR)\user32.obj \
		$(OBJDIR)\version.obj \
		$(OBJDIR)\winmm.obj \

ASMS = $(OBJECTS:.obj=.S)

_all: $(OBJDIR) $(TARGET)

$(OBJECTS) : $(ASMS)

$(TARGET): $(OBJECTS)
	@echo -- collect -- $@
	@link /lib -nologo -machine:$(CPU) -out:$(TARGET) $(OBJECTS) 
	
{}.Xi{$(OBJDIR)}.S:
	@echo generating $(*F).S
	@python xigentor.py $< $(OBJDIR)\$(*F).S $(*F)
	
{$(OBJDIR)}.S{$(OBJDIR)}.obj:
	@echo compiling $(*F).S
	@$(AS) $(_M64) $< -o$(OBJDIR)\$(*F).obj 

clean:
	-erase $(OBJECTS)
	-erase $(ASMS)
	-erase $(TARGET)
	-erase *.H *.S

