# -*- coding: cp1251 -*-
"""
(c)2008 Alexey Sudachen, alexey@sudachen.name

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
"""

import sys, os, os.path, mmap
from ctypes import Structure,Union,sizeof,\
        c_char,c_wchar,c_byte,c_char_p,c_void_p,\
        c_short,c_ushort,\
        c_long,c_ulong,c_int,c_uint,\
        POINTER, cast, string_at,\
        addressof

BYTE = c_byte
WORD = c_ushort
DWORD = c_ulong
WCHAR = c_wchar
UINT = c_uint
INT = c_int
BOOLEAN = BYTE
BOOL = c_long
ULONG = c_ulong
LONG = c_long
USHORT = c_ushort
SHORT = c_short

class Bsection(object):

    __slots__ = ['flags','virtual_address','virtual_size','pointer_to_data','size_of_data','name']

    ACCESS_EXECUTE = 0x8
    ACCESS_WRITE   = 0x2
    ACCESS_XWRITE  = 0xa

    def __init__(self,flags,name,va,vsize,dta,dsize):
        self.virtual_address = va
        self.virtual_size = vsize
        self.pointer_to_data = dta
        self.size_of_data = dsize
        self.name = name
        self.flags = flags

    def __str__(self):
        return "SECTION{%s %8s %04x/%04x <= %04x/%04x}" % (
            self.access(), self.name,
            self.virtual_address,self.virtual_size,
            self.pointer_to_data,self.size_of_data)

    def access(self):
        x = self.flags & 0x0f
        if x == Bsection.ACCESS_WRITE:  return 'W'
        if x == Bsection.ACCESS_XWRITE: return 'X'
        if x == Bsection.ACCESS_EXECUTE:return 'E'
        return 'R'

class Bsymbol(object):

    __slots__ = ['address','name']

    def __init__(self,name,addr):
        self.name = name
        self.address = int(addr)

    def __str__(self):
        return 'SYMBOL{%s,%08x}' % (self.name,self.address)

IMAGE_DOS_SIGNATURE = 0x5A4D      # MZ
IMAGE_OS2_SIGNATURE = 0x454E      # NE
IMAGE_OS2_SIGNATURE_LE = 0x454C   # LE
IMAGE_VXD_SIGNATURE = 0x454C      # LE
IMAGE_NT_SIGNATURE  = 0x00004550  # PE00

class IMAGE_DOS_HEADER(Structure):
    _fields_ = [
        ('e_magic',WORD),
        ('e_cblp',WORD),
        ('e_cp',WORD),
        ('e_crlc',WORD),
        ('e_cparhdr',WORD),
        ('e_minalloc',WORD),
        ('e_maxalloc',WORD),
        ('e_ss',WORD),
        ('e_sp',WORD),
        ('e_csum',WORD),
        ('e_ip',WORD),
        ('e_cs',WORD),
        ('e_lfarlc',WORD),
        ('e_ovno',WORD),
        ('e_res',WORD*4),
        ('e_oemid',WORD),
        ('e_oeminfo',WORD),
        ('e_res2',WORD*10),
        ('e_lfanew',LONG),
        ]

IMAGE_SIZEOF_DOS_HEADER = 64

class IMAGE_FILE_HEADER(Structure):
    _fields_ = [
        ('Machine',WORD),
        ('NumberOfSections',WORD),
        ('TimeDateStamp',DWORD),
        ('PointerToSymbolTable',DWORD),
        ('NumberOfSymbols',DWORD),
        ('SizeOfOptionalHeader',WORD),
        ('Characteristics',WORD),
        ]

IMAGE_SIZEOF_FILE_HEADER    = 20

image_file_carhs_values = [
    ('IMAGE_FILE_RELOCS_STRIPPED'           ,0x0001),  # Relocation info stripped from file.
    ('IMAGE_FILE_EXECUTABLE_IMAGE'          ,0x0002),  # File is executable  (i.e. no unresolved externel references),.
    ('IMAGE_FILE_LINE_NUMS_STRIPPED'        ,0x0004),  # Line nunbers stripped from file.
    ('IMAGE_FILE_LOCAL_SYMS_STRIPPED'       ,0x0008),  # Local symbols stripped from file.
    ('IMAGE_FILE_AGGRESIVE_WS_TRIM'         ,0x0010),  # Agressively trim working set
    ('IMAGE_FILE_LARGE_ADDRESS_AWARE'       ,0x0020),  # App can handle >2gb addresses
    ('IMAGE_FILE_BYTES_REVERSED_LO'         ,0x0080),  # Bytes of machine word are reversed.
    ('IMAGE_FILE_32BIT_MACHINE'             ,0x0100),  # 32 bit word machine.
    ('IMAGE_FILE_DEBUG_STRIPPED'            ,0x0200),  # Debugging info stripped from file in .DBG file
    ('IMAGE_FILE_REMOVABLE_RUN_FROM_SWAP'   ,0x0400),  # If Image is on removable media, copy and run from the swap file.
    ('IMAGE_FILE_NET_RUN_FROM_SWAP'         ,0x0800),  # If Image is on Net, copy and run from the swap file.
    ('IMAGE_FILE_SYSTEM'                    ,0x1000),  # System File.
    ('IMAGE_FILE_DLL'                       ,0x2000),  # File is a DLL.
    ('IMAGE_FILE_UP_SYSTEM_ONLY'            ,0x4000),  # File should only be run on a UP machine
    ('IMAGE_FILE_BYTES_REVERSED_HI'         ,0x8000),  # Bytes of machine word are reversed.
    ]
image_file_carhs_map = dict( map( lambda x: (x[1],x[0]), image_file_carhs_values) )
for i,j in image_file_carhs_values: globals()[i] = j

image_file_machine_values = [
    ('IMAGE_FILE_MACHINE_UNKNOWN'           , 0),
    ('IMAGE_FILE_MACHINE_I386'              , 0x014c),  # Intel 386.
    ('IMAGE_FILE_MACHINE_R3000'             , 0x0162),  # MIPS little-endian, 0x160 big-endian
    ('IMAGE_FILE_MACHINE_R4000'             , 0x0166),  # MIPS little-endian
    ('IMAGE_FILE_MACHINE_R10000'            , 0x0168),  # MIPS little-endian
    ('IMAGE_FILE_MACHINE_WCEMIPSV2'         , 0x0169),  # MIPS little-endian WCE v2
    ('IMAGE_FILE_MACHINE_ALPHA'             , 0x0184),  # Alpha_AXP
    ('IMAGE_FILE_MACHINE_SH3'               , 0x01a2),  # SH3 little-endian
    ('IMAGE_FILE_MACHINE_SH3DSP'            , 0x01a3),
    ('IMAGE_FILE_MACHINE_SH3E'              , 0x01a4),  # SH3E little-endian
    ('IMAGE_FILE_MACHINE_SH4'               , 0x01a6),  # SH4 little-endian
    ('IMAGE_FILE_MACHINE_SH5'               , 0x01a8),  # SH5
    ('IMAGE_FILE_MACHINE_ARM'               , 0x01c0),  # ARM Little-Endian
    ('IMAGE_FILE_MACHINE_THUMB'             , 0x01c2),
    ('IMAGE_FILE_MACHINE_AM33'              , 0x01d3),
    ('IMAGE_FILE_MACHINE_POWERPC'           , 0x01F0),  # IBM PowerPC Little-Endian
    ('IMAGE_FILE_MACHINE_POWERPCFP'         , 0x01f1),
    ('IMAGE_FILE_MACHINE_IA64'              , 0x0200),  # Intel 64
    ('IMAGE_FILE_MACHINE_MIPS16'            , 0x0266),  # MIPS
    ('IMAGE_FILE_MACHINE_ALPHA64'           , 0x0284),  # ALPHA64
    ('IMAGE_FILE_MACHINE_MIPSFPU'           , 0x0366),  # MIPS
    ('IMAGE_FILE_MACHINE_MIPSFPU16'         , 0x0466),  # MIPS
    ('IMAGE_FILE_MACHINE_AXP64'             , 0x0284),  # IMAGE_FILE_MACHINE_ALPHA64
    ('IMAGE_FILE_MACHINE_TRICORE'           , 0x0520),  # Infineon
    ('IMAGE_FILE_MACHINE_CEF'               , 0x0CEF),
    ('IMAGE_FILE_MACHINE_EBC'               , 0x0EBC),  # EFI Byte Code
    ('IMAGE_FILE_MACHINE_AMD64'             , 0x8664),  # AMD64 (K8),
    ('IMAGE_FILE_MACHINE_M32R'              , 0x9041),  # M32R little-endian
    ('IMAGE_FILE_MACHINE_CEE'               , 0xC0EE),
    ]
image_file_machine_map = dict( map( lambda x: (x[1],x[0]), image_file_machine_values) )
for i,j in image_file_machine_values: globals()[i] = j

class IMAGE_DATA_DIRECTORY(Structure):
    _fields_ = [
        ('VirtualAddress',DWORD),
        ('Size',DWORD),
        ]

IMAGE_NUMBEROF_DIRECTORY_ENTRIES = 16

image_dictionary_names_values = [
    ('IMAGE_DIRECTORY_ENTRY_EXPORT',        0),
    ('IMAGE_DIRECTORY_ENTRY_IMPORT',        1),
    ('IMAGE_DIRECTORY_ENTRY_RESOURCE',      2),
    ('IMAGE_DIRECTORY_ENTRY_EXCEPTION',     3),
    ('IMAGE_DIRECTORY_ENTRY_SECURITY',      4),
    ('IMAGE_DIRECTORY_ENTRY_BASERELOC',     5),
    ('IMAGE_DIRECTORY_ENTRY_DEBUG',         6),
    ('IMAGE_DIRECTORY_ENTRY_COPYRIGHT',     7),
    ('IMAGE_DIRECTORY_ENTRY_GLOBALPTR',     8),
    ('IMAGE_DIRECTORY_ENTRY_TLS',           9),
    ('IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG',   10),
    ('IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT',  11),
    ('IMAGE_DIRECTORY_ENTRY_IAT',           12),
    ('IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT',  13),
    ('IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR',14),
    ('IMAGE_DIRECTORY_ENTRY_RESERVED',      15),
    ]

image_dictionary_names_map = dict( map( lambda x: (x[1],x[0]), image_dictionary_names_values ) )
for i,j in image_dictionary_names_values: globals()[i] = j

class IMAGE_OPTIONAL_HEADER(Structure):
    _fields_ = [
        # Standard fields.
        ('Magic',WORD),
        ('MajorLinkerVersion',BYTE),
        ('MinorLinkerVersion',BYTE),
        ('SizeOfCode',DWORD),
        ('SizeOfInitializedData',DWORD),
        ('SizeOfUninitializedData',DWORD),
        ('AddressOfEntryPoint',DWORD),
        ('BaseOfCode',DWORD),
        ('BaseOfData',DWORD),
        # NT additional fields.
        ('ImageBase',DWORD),
        ('SectionAlignment',DWORD),
        ('FileAlignment',DWORD),
        ('MajorOperatingSystemVersion',WORD),
        ('MinorOperatingSystemVersion',WORD),
        ('MajorImageVersion',WORD),
        ('MinorImageVersion',WORD),
        ('MajorSubsystemVersion',WORD),
        ('MinorSubsystemVersion',WORD),
        ('Win32VersionValue',DWORD),
        ('SizeOfImage',DWORD),
        ('SizeOfHeaders',DWORD),
        ('CheckSum',DWORD),
        ('Subsystem',WORD),
        ('DllCharacteristics',WORD),
        ('SizeOfStackReserve',DWORD),
        ('SizeOfStackCommit',DWORD),
        ('SizeOfHeapReserve',DWORD),
        ('SizeOfHeapCommit',DWORD),
        ('LoaderFlags',DWORD),
        ('NumberOfRvaAndSizes',DWORD),
        ('DataDirectory',IMAGE_DATA_DIRECTORY*IMAGE_NUMBEROF_DIRECTORY_ENTRIES),
        ]

class IMAGE_NT_HEADERS(Structure):
    _fields_ = [
        ('Signature',DWORD),
        ('FileHeader',IMAGE_FILE_HEADER),
        ('OptionalHeader',IMAGE_OPTIONAL_HEADER),
    ]

IMAGE_SIZEOF_SHORT_NAME = 8

class IMAGE_SECTION_HEADER(Structure):
    _fields_ = [
        ('Name',c_char*IMAGE_SIZEOF_SHORT_NAME),
        ('VirtualSize',DWORD),
        ('VirtualAddress',DWORD),
        ('SizeOfRawData',DWORD),
        ('PointerToRawData',DWORD),
        ('PointerToRelocations',DWORD),
        ('PointerToLinenumbers',DWORD),
        ('NumberOfRelocations',WORD),
        ('NumberOfLinenumbers',WORD),
        ('Characteristics',DWORD),
    ]

class IMAGE_EXPORT_DIRECTORY(Structure):
    _fields_ = [
        ('Characteristics',DWORD),
        ('TimeDateStamp',DWORD),
        ('MajorVersion',WORD),
        ('MinorVersion',WORD),
        ('Name',DWORD),
        ('Base',DWORD),
        ('NumberOfFunctions',DWORD),
        ('NumberOfNames',DWORD),
        ('AddressOfFunctions',DWORD),     # RVA from base of image
        ('AddressOfNames',DWORD),         # RVA from base of image
        ('AddressOfNameOrdinals',DWORD),  # RVA from base of image
    ]

class IMAGE_IMPORT_BY_NAME(Structure):
    _fields_ = [
        ('Hint',WORD),
        ('Name',c_char*256)
    ]


class IMAGE_THUNK_DATA(Union):
    _fields_ = [
        ('ForwarderString', DWORD),
        ('Function',DWORD),
        ('Ordinal',DWORD),
        ('AddressOfData',DWORD),
    ]

class IMAGE_IMPORT_DESCRIPTOR(Structure):
    _fields_ = [
        ('OriginalFirstThunk',DWORD),
        ('TimeDateStamp',DWORD),
        ('ForwarderChain',DWORD),
        ('Name',DWORD),
        ('FirstThunk',DWORD),
    ]

def translate_chars(x):
    y = 0
    x = x >> 24 & ~0x040
    if x & 0x080: y = y | Bsection.ACCESS_WRITE
    if x & 0x020: y = y | Bsection.ACCESS_EXECUTE
    return y

def make_b_section(s):
    return Bsection(translate_chars(s.Characteristics),
                    s.Name,
                    s.VirtualAddress,s.VirtualSize,
                    s.PointerToRawData,s.SizeOfRawData)

class Bfile(object):

    def __init__(self,mem):
        try:
            dsh = IMAGE_DOS_HEADER.from_address(mem)
            if dsh.e_magic != IMAGE_DOS_SIGNATURE: raise Exception('file is not DOS/PE image')
            nth = IMAGE_NT_HEADERS.from_address(mem+dsh.e_lfanew)
            if nth.Signature != IMAGE_NT_SIGNATURE: raise Exception('file is not PE image')
            self._basemem = mem
            self.dos_header = dsh
            self.nt_headers = nth
            self.image_base = self.nt_headers.OptionalHeader.ImageBase
        except:
            raise

    def close(self):
        if self._basemem:
            unmmap_file(self._basemem)
            self._basename = 0
            self.nt_headers = None
            self.dos_header = None

    def align_data_ptr(self,p):
        fa = self.nt_headers.OptionalHeader.FileAlignment - 1
        return ( p + fa ) & ~fa

    def rva_to_va(self,rva):
        return rva+self.get_image_base()

    def get_nt_first_section_offset(self):
        return self.dos_header.e_lfanew + 4 + \
            sizeof(IMAGE_FILE_HEADER) + \
            self.nt_headers.FileHeader.SizeOfOptionalHeader

    def get_nt_sections(self):
        return (IMAGE_SECTION_HEADER*self.nt_headers.FileHeader.NumberOfSections).\
            from_address(self._basemem + self.get_nt_first_section_offset())

    def rva_to_nt_section(self,rva):
        for sect in self.get_nt_sections():
            if rva >= sect.VirtualAddress and rva < sect.VirtualAddress+max(sect.VirtualSize,sect.SizeOfRawData):
                return sect
        return None

    def rva_to_data(self,rva):
        sec = self.rva_to_nt_section(rva)
        if sec:
            return self._basemem + (rva - sec.VirtualAddress + sec.PointerToRawData)
        raise Exception('failed to rva_to_data convertion %08x, there is no useful section' % rva)

    def get_image_base(self):
        return self.image_base

    def get_entry_point_va(self):
        return self.get_image_base()+self.nt_headers.OptionalHeader.AddressOfEntryPoint

    def get_entry_point_rva(self):
        return self.nt_headers.OptionalHeader.AddressOfEntryPoint

    def get_sections_count(self):
        return self.nt_headers.FileHeader.NumberOfSections

    def get_section(self,no):
        return make_b_section(self.get_nt_sections()[no])

    def enumerate_sections(self):
        for s in self.get_nt_sections():
            yield make_b_section(s)

    def get_nt_exports(self):
        exports_rva = self.nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress
        if exports_rva:
            exports = self.rva_to_data(exports_rva)
            return IMAGE_EXPORT_DIRECTORY.from_address(exports)
        return None

    def enumerate_exports(self):
        exports = self.get_nt_exports()
        if exports and exports.NumberOfNames:
            names = (DWORD*exports.NumberOfNames).from_address(self.rva_to_data(exports.AddressOfNames))
            funcs = (DWORD*exports.NumberOfFunctions).from_address(self.rva_to_data(exports.AddressOfFunctions))
            ords  = (WORD*exports.NumberOfNames).from_address(self.rva_to_data(exports.AddressOfNameOrdinals))
            for i in range(exports.NumberOfNames):
                yield Bsymbol((c_char*256).from_address(self.rva_to_data(names[i])).value,\
                       funcs[ords[i]]+self.get_image_base())

    def get_headers(self):
        return self.nt_headers

    def get_nt_imports(self):
        l = []
        imports_rva = self.nt_headers.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
        if imports_rva:
            p = self.rva_to_data(imports_rva)
            i = IMAGE_IMPORT_DESCRIPTOR.from_address(p)
            while i.FirstThunk and i.Name:
                name = c_char_p(self.rva_to_data(i.Name)).value
                j = i.OriginalFirstThunk
                if not j: j = i.FirstThunk
                l.append( (name, self.rva_to_data(j), self.rva_to_va(i.FirstThunk)) )
                p += sizeof(IMAGE_IMPORT_DESCRIPTOR)
                i = IMAGE_IMPORT_DESCRIPTOR.from_address(p)
        return l

    def enumerate_dependences(self):
        for n,p,f in self.get_nt_imports():
            yield n

    def enumerate_dependences_and_count(self):
        def count_imports(p):
            i = 0;
            while p[i]: i+=1
            return i
        for n,p,f in self.get_nt_imports():
            yield (n, count_imports(cast(p,POINTER(DWORD))))

    def enumerate_imports(self):
        for n,p,f in self.get_nt_imports():
            n = os.path.splitext(n)[0].lower()
            p = cast(p,POINTER(DWORD)); i = 0
            while p[i]:
                a = p[i]
                if a & 0x80000000:
                    yield Bsymbol(n+'!#%d'%(a&0x0ffff),i*4+f)
                else:
                    a = cast(self.rva_to_data(a)+2,c_char_p)
                    yield Bsymbol(n+'!'+a.value,i*4+f)
                i += 1

    def get_bytes_va(self,va,count):
        return string_at(self.rva_to_data(va-self.get_image_base()),count)

    def get_bytes_dta(self,dta,count):
        return string_at(self.get_base_address()+dta,count)

    def get_section_data(self,no):
        sect = self.get_sections()
        return (c_byte*sect[no].SizeOfRawData).from_address(self._basemem+sect[no].PointerToRawData)

    def get_mmap_va(self,va,count=0):
        return self.rva_to_data(va-self.image_base_address())

    def get_mmap_rva(self,rva,count=0):
        return self.rva_to_data(rva)

    def get_mmap_offs(self,offs,count=0):
        return self.get_base_address()+offs

    def enumerate_symbols(self):
        for i in self.enumerate_imports():
            yield i
        for i in self.enumerate_exports():
            yield i

    def find_section(self,va):
        return make_b_section(self.rva_to_nt_section(va - self.get_image_base()))

    def find_section_and_offset(self,va):
        bsect = self.find_section(va)
        return bsect, va-bsect.virtual_address-self.get_image_base()

    def get_base_address(self):
        return self._basemem

    def data_to_va(self,p):
        for sect in self.get_sections():
            if p >= sect.PointerToRawData and p < sect.PointerToRawData+sect.SizeOfRawData:
                return (p-sect.PointerToRawData)+sect.VirtualAddress+self.get_image_base()
        return None

    def fill_valuable_fields(self):

        def format_subsys(x):
            if not x: return 'unknown'
            if x == 1: return 'native'
            if x == 2: return 'windows'
            if x == 3: return 'console'
            if x == 7: return 'posix'
            return str(x)

        o = self.nt_headers.OptionalHeader
        f = self.nt_headers.FileHeader

        fields = [
            ('LINKER','%d.%d'%(o.MajorLinkerVersion,o.MinorLinkerVersion)),
            #('ENTRYPOINT','%08x/%s'%(o.AddressOfEntryPoint+o.ImageBase,eps.Name)),
            ('ENTRYPOINT','%08x'%(o.AddressOfEntryPoint)),
            ('IMAGEBASE','%08x'%o.ImageBase),
            ('ALIGNMENT','%x/%x'%(o.SectionAlignment,o.FileAlignment)),
            ('SUBSYTEM','%s'%format_subsys(o.Subsystem)),
            ('IMAGESIZE','%08x'%o.SizeOfImage),
            ('HEADERSIZE','%08x'%o.SizeOfHeaders),
            ('CHECKSUM','%08x'%o.CheckSum),
            ('STACKSIZE','%x/%x'%(o.SizeOfStackCommit,o.SizeOfStackReserve)),
            ('HEAPSIZE','%x/%x'%(o.SizeOfHeapCommit,o.SizeOfHeapReserve)),
            ]

        fields = [ ('%'+str(max(map(lambda x: len(x[0]),fields)))+'s: %s')%i for i in fields ]

        def fromat_flags(x):
            l = []
            if x & 0x0001 : l.append('FIXED')
            if x & 0x0002 and not x & 0x2000: l.append('EXECUTABLE')
            #if x & 0x0010 : l.append('trim the working set')
            #if x & 0x0020 : l.append('more then 2G')
            if x & 0xc00  : l.append('SWAPPED')
            if x & 0x4000 : l.append('UNIPROCESSOR')
            if x & 0x2000 : l.append('DLL')
            if l: l = ['.'] + l
            return l

        fields += fromat_flags(f.Characteristics)

        return fields

    def fill_specific_fields(self):

        o = self.nt_headers.OptionalHeader
        f = self.nt_headers.FileHeader
        sects = []

        for i,j in [('IMPORT','IMPORTS:'),('EXPORT','EXPORTS:'),('BASERELOC','RELOCS: ')]:
            if o.DataDirectory[globals()['IMAGE_DIRECTORY_ENTRY_'+i]].VirtualAddress:
                d = o.DataDirectory[globals()['IMAGE_DIRECTORY_ENTRY_'+i]]
                s,f = self.find_section_and_offset(d.VirtualAddress+o.ImageBase)
                sects += [j+" %08x/%04x (%s+%04x)" % (d.VirtualAddress,d.Size,s.name,f)]

        return sects

    def fill_full_headers(self):
        o = self.nt_headers.OptionalHeader
        f = self.nt_headers.FileHeader

        out = []

        def format_subsys(x):
            if not x: return 'unknown'
            if x == 1: return 'native'
            if x == 2: return 'windows'
            if x == 3: return 'console'
            if x == 7: return 'posix'
            return str(x)

        def align(x,y):
            return ((x + y -1 ) / y) * y

        out.append('OPTIONAL HEADER VALUES')
        out.append('%16x magic' % o.Magic)
        out.append('%16s linker version' % ('%d.%d' % (o.MajorLinkerVersion,o.MinorLinkerVersion)))
        out.append('%16x size of code' % o.SizeOfCode)
        out.append('%16x size of initialized data' % o.SizeOfInitializedData)
        out.append('%16x size of uninitialized data' % o.SizeOfUninitializedData)
        out.append('%16x entry point (%08x)'  % (o.AddressOfEntryPoint, o.AddressOfEntryPoint+o.ImageBase))
        out.append('%16x base of code' % o.BaseOfCode)
        out.append('%16x base of data' % o.BaseOfData)
        out.append('%16x image base (%08x to %08x)'  % (o.ImageBase,o.ImageBase,align(o.ImageBase+o.SizeOfImage,o.SectionAlignment)-1))
        out.append('%16x section alignment' % o.SectionAlignment)
        out.append('%16x file alignment' % o.FileAlignment)
        out.append('%16s operating system version' % ('%d.%d' % (o.MajorOperatingSystemVersion,o.MinorOperatingSystemVersion)))
        out.append('%16s image version' % ('%d.%d' % (o.MajorImageVersion,o.MinorImageVersion)))
        out.append('%16s subsystem version' % ('%d.%d' % (o.MajorSubsystemVersion,o.MinorSubsystemVersion)))
        out.append('%16x Win32 version' % o.Win32VersionValue)
        out.append('%16x size of image' % o.SizeOfImage)
        out.append('%16x size of headers' % o.SizeOfHeaders)
        out.append('%16x checksum' % o.CheckSum)
        out.append('%16x subsystem (%s)' % (o.Subsystem, format_subsys(o.Subsystem)))
        out.append('%16x DLL characteristics' % o.DllCharacteristics)
        out.append('%16x size of stack reserve' % o.SizeOfStackReserve)
        out.append('%16x size of stack commit' % o.SizeOfStackCommit)
        out.append('%16x size of heap reserve' % o.SizeOfHeapReserve)
        out.append('%16x size of heap commit' % o.SizeOfHeapCommit)
        out.append('%16x loader flags' % o.LoaderFlags)
        out.append('%16x number of directories' % o.NumberOfRvaAndSizes)
        out.append('%16x [%8x] RVA [size] of Export Directory' % (o.DataDirectory[0].VirtualAddress,o.DataDirectory[0].Size))
        out.append('%16x [%8x] RVA [size] of Import Directory' % (o.DataDirectory[1].VirtualAddress,o.DataDirectory[1].Size))
        out.append('%16x [%8x] RVA [size] of Resource Directory' % (o.DataDirectory[2].VirtualAddress,o.DataDirectory[2].Size))
        out.append('%16x [%8x] RVA [size] of Exception Directory' % (o.DataDirectory[3].VirtualAddress,o.DataDirectory[3].Size))
        out.append('%16x [%8x] RVA [size] of Certificates Directory' % (o.DataDirectory[4].VirtualAddress,o.DataDirectory[4].Size))
        out.append('%16x [%8x] RVA [size] of Base Relocation Directory' % (o.DataDirectory[5].VirtualAddress,o.DataDirectory[5].Size))
        out.append('%16x [%8x] RVA [size] of Debug Directory' % (o.DataDirectory[6].VirtualAddress,o.DataDirectory[6].Size))
        out.append('%16x [%8x] RVA [size] of Architecture Directory' % (o.DataDirectory[7].VirtualAddress,o.DataDirectory[7].Size))
        out.append('%16x [%8x] RVA [size] of Global Pointer Directory' % (o.DataDirectory[8].VirtualAddress,o.DataDirectory[8].Size))
        out.append('%16x [%8x] RVA [size] of Thread Storage Directory' % (o.DataDirectory[9].VirtualAddress,o.DataDirectory[9].Size))
        out.append('%16x [%8x] RVA [size] of Load Configuration Directory' % (o.DataDirectory[10].VirtualAddress,o.DataDirectory[10].Size))
        out.append('%16x [%8x] RVA [size] of Bound Import Directory' % (o.DataDirectory[11].VirtualAddress,o.DataDirectory[11].Size))
        out.append('%16x [%8x] RVA [size] of Import Address Table Directory' % (o.DataDirectory[12].VirtualAddress,o.DataDirectory[12].Size))
        out.append('%16x [%8x] RVA [size] of Delay Import Directory' % (o.DataDirectory[13].VirtualAddress,o.DataDirectory[13].Size))
        out.append('%16x [%8x] RVA [size] of COM Descriptor Directory' % (o.DataDirectory[14].VirtualAddress,o.DataDirectory[14].Size))
        out.append('%16x [%8x] RVA [size] of Reserved Directory' % (o.DataDirectory[15].VirtualAddress,o.DataDirectory[15].Size))
        out.append('')

        n = 0
        for i in self.get_nt_sections():
            n += 1
            out.append('SECTION HEADER #%d' % n)
            out.append('%8s name' % i.Name)
            out.append('%8x virtual size'  % i.VirtualSize)
            out.append('%8x virtual address (%08x to %08x)' % \
                (i.VirtualAddress,
                    i.VirtualAddress+self.get_image_base(),
                    i.VirtualAddress+self.get_image_base()+
                        align(max(i.VirtualSize,i.SizeOfRawData),o.SectionAlignment)-1))
            out.append('%8x size of raw data' % i.SizeOfRawData)
            out.append('%8x file pointer to raw data' % i.PointerToRawData)
            out.append('%8x file pointer to relocation table' % i.PointerToRelocations)
            out.append('%8x file pointer to line numbers' % i.PointerToLinenumbers)
            out.append('%8x number of relocations' % i.NumberOfRelocations)
            out.append('%8x number of line numbers' % i.NumberOfLinenumbers)
            out.append('%8x flags' % i.Characteristics)
            out.append('')

        return out


class PEfile(Bfile):

    def __init__(self,fname):
        f = open(fname,"r+b")
        self.mem = mmap.mmap(f.fileno(),0)
        self.memS = self.mem[0:]
        Bfile.__init__(self,cast(self.memS,c_void_p).value)
