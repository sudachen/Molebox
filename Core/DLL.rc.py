import re,time,os

RC_PROTO = r"""
1 VERSIONINFO
 FILEVERSION %%VERSION%%,0,0,%%BUILD%%
 PRODUCTVERSION %%VERSION%%,0,0,%%BUILD%%
 FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x40004L
 FILETYPE 0x2L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "000004b0"
        BEGIN
            VALUE "Comments", "\0"
            VALUE "CompanyName", "DesaNova Ltda.\0"
            VALUE "FileDescription", "MOLEBOX REVISION IV VBOXCORE (STEELBOX)\0"
            VALUE "FileVersion", "%%VERSION%%, 0, 0, %%BUILD%%\0"
            VALUE "InternalName", "\0"
            VALUE "LegalCopyright", "Copyright (c) 2008-2009, Alexey Sudachen, DesaNova Ltda.\0"
            VALUE "LegalTrademarks", "\0"
            VALUE "OriginalFilename", "\0"
            VALUE "PrivateBuild", "\0"
            VALUE "ProductName", "\0"
            VALUE "ProductVersion", "%%VERSION%%, 0, 0, %%BUILD%%\0"
            VALUE "SpecialBuild", "\0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x0, 1200
    END
END
"""

VERSION = 4

b = int(open("../version.build","r").readline())

if os.getenv('BUILDBOT_VERSION_INCREMENT',None):
    b += 1
    open("../version.build","w+").write(str(b))

l = RC_PROTO.split('\n')
o = open("DLL.rc","w+t")
for i in l:
    i = re.sub( '%%BUILD%%', str(b), i)
    i = re.sub( '%%VERSION%%', str(VERSION), i)
    o.write("%s\n" % i)
o.close()

o = open("version.c","w+")
o.write("#include <time.h>\n");
o.write("int _MAJOR_VERSION = %d;\n" % VERSION)
o.write("int _BUILD_NUMBER = %d;\n" % b)
t = time.mktime(time.strptime("30 11 2011","%d %m %Y"))
print time.asctime(time.localtime(t))
o.write("time_t _BUILD_TIME = %.0f;\n" % t )
o.close()
