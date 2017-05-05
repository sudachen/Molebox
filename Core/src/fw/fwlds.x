OUTPUT_FORMAT(pei-i386)
ENTRY(_Start)
SEARCH_DIR("/mingw/mingw32/lib"); SEARCH_DIR("/mingw/lib"); SEARCH_DIR("/usr/local/lib"); SEARCH_DIR("/lib"); SEARCH_DIR("/usr/lib");
SECTIONS
{
  . = SIZEOF_HEADERS;
  . = ALIGN(__section_alignment__);
  __extra_base__ = __image_base__ + ( __section_alignment__ < 0x1000 ? . : __section_alignment__ );
  .extra __extra_base__ :
  {
    *(.text$start)
    *(.text$ndes)
    *(.text$xor)
    *(.text$aux)
    *(.text$code)
  }
  /DISCARD/ :
  {
    *(.text)
    *(.reloc)
    *(.edata)
    *(.bss)
    *(COMMON)
    *(.rsrc)
    *(.idata*)
    *(.data*)
    *(.rdata*)
    *(.END)
  }
}
