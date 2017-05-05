import os, os.path, sys, struct, zlib
from ctypes import Structure,Union,sizeof,c_char,c_byte,c_char_p,c_long,POINTER, cast, byref, addressof, string_at
from ctypes.wintypes import *
from tempfile import mkstemp

os.chdir(os.path.abspath(os.path.dirname(__file__)))

def myrand(s):
    S = s
    while True:
        S = (((1664525L*S)%(1L<<32)) + 1013904223L)%(1L<<32)
        yield S


from pefile import *
import _lzss

SIGN0 = 0x45455453
SIGN1 = 0x584F424C

class EMBEDEDHDR(Structure):
    _fields_ = [
        ('sign0',DWORD),
        ('sign1',DWORD),
        ('build_number',DWORD),
        ('imports',DWORD),
        ('imports_size',DWORD),
        ('relocs',DWORD),
        ('relocs_size',DWORD),
        ('image_base',DWORD),
        ('flags',DWORD),
        ('inity',DWORD),
        ('install',DWORD),
        ('sdkinit',DWORD),
        ('inject',DWORD),
        ]

eh = EMBEDEDHDR()
pef  = PEfile(sys.argv[1])
cor  = open(sys.argv[3],"w+b")

S_text = None
S_expo = None
S_relo = None
for i in pef.get_nt_sections():
    if i.Name == '.text':  S_text = i
    if i.Name == '.expo':  S_expo = i
    if i.Name == '.reloc': S_relo = i

eh.sign0 = SIGN0
eh.sign1 = SIGN1
eh.image_base = pef.get_headers().OptionalHeader.ImageBase
eh.build_number = 6000 #int(open("DLL.build","r").readline())
eh.relocs_size = pef.get_headers().OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size
eh.relocs = len(EMBEDEDHDR._fields_)*4

byteS  = pef.get_bytes_va( S_text.VirtualAddress + eh.image_base, S_text.SizeOfRawData )
relocS = pef.get_bytes_va( S_relo.VirtualAddress + eh.image_base, eh.relocs_size )

if len(sys.argv) > 4:
    crc = zlib.adler32(byteS,0)
    print "ADR:%08x, SIZ:%08x, CRC:%08x"%(S_text.VirtualAddress,S_text.SizeOfRawData,crc)
    #rnd = myrand(crc)
    #relocS = ''.join([chr(ord(i)^(rnd.next()&0x0ff)) for i in relocS])
    #relocS = ''.join([chr(ord(i)^(rnd.next()&0x0ff)) for i in relocS])

cor.seek(len(EMBEDEDHDR._fields_)*4)
cor.write(relocS)
cor.seek(S_text.VirtualAddress)
cor.write(byteS)

eh.imports = 0 #pef.get_headers().OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress
eh.imports_size = 0 #pef.get_headers().OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size

for i in pef.enumerate_exports():
    if i.name == '_Inity@16': eh.inity = i.address - eh.image_base
    if i.name == '_Install@16': eh.install = i.address - eh.image_base
    if i.name == '_SDK_Inject@20': eh.inject = i.address - eh.image_base

s = string_at(addressof(eh),len(EMBEDEDHDR._fields_)*4)
print repr(s)
cor.seek(0)
cor.write(s)
cor.close()

corS = open(sys.argv[3],"r+b").read()
corS = corS
corS1 = _lzss.compress(corS)

if corS <= corS1:
    print "could not compress image, failed"
    exit(-1)
print len(corS), '->', len(corS1)

cor = open(sys.argv[2],"w+b")
#cor.write(struct.pack("I",len(corS)))
cor.write(corS1)
cor.write("\0\0\0\0\0\0\0\0")
cor.close()
