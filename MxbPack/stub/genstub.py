#!/usr/bin/env python
import sys,os,os.path,zlib,struct,time,random
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import mkrotor

PYTHON = "C:\\Python27\\python.exe"
GCC = "C:\\Opt\\C++\\gcc-3.4.5\\bin\\i386-mingw32-gcc.exe"
STRIP = "C:\\Opt\\C++\\gcc-3.4.5\\bin\\i386-mingw32-strip.exe"

os.chdir(os.path.dirname(os.path.abspath(__file__)))

Sname = sys.argv[1]
defs = []

for i in sys.argv:
    if i.startswith('-D'):
        defs.append(i)

if len(sys.argv) > 2:
    buildway = sys.argv[2]
else:
    buildway = 'rcl'

if buildway not in ('gcc','cl','rcl'):
    buildway = 'rcl'

if len(sys.argv) > 3:
    N = int(sys.argv[3])
else:
    N = 1

print 'generating stub '+Sname+".inl"

def unlink(S):
    if os.path.exists(S):
        os.unlink(S)

def randombytes(n):
    bits = os.urandom(n)
    q = 0L
    while bits:
        q = (q<<8) | ord(bits[0])
        bits = bits[1:]
    return q;


def exe(cmd):
    r = os.system(cmd)
    print '## '+str(r)+' <= '+cmd
    if r != 0:
        raise Exception('failed at exe('+str(r)+'):'+cmd)

def Q(Sname):
    unlink("../metaproc.c")
    unlink("../metaproc1.c")
    unlink("../"+Sname+".inl")
    unlink("../"+Sname+".BIN")
    unlink("../"+Sname+".c")
    
    SID = int(random.random()*65535)
    rotor = mkrotor.gen_rotor(SID)
    
    rH = open("rotor.h","w+")
    rH.write("#define _ROTOR_SID %d\n"%SID)
    rH.write("#define _ROTOR %s\n\n"%rotor)
    rH.close()
    
    if os.path.exists('image00400000.pdb'):
        os.unlink('image00400000.pdb')
    if buildway == 'gcc':
        exe(GCC+' -I ../../include -E metastab.c -D _METAPROC -D__stdcall=__stdcall '+' '.join(defs)+' -D_RANDOM='+str(randombytes(4))+' > ../metaproc.c')
        exe(PYTHON+' metaproc.py ../metaproc.c '+str(SID)+' > '+Sname+".c ")
        exe(GCC+' -D__stcall=__attribute__((__stdcall__)) '+' '.join(defs)+" "+Sname+".c -O1 -w -Wl,-s -Wl,-S -Wl,--strip-discarded -Wl,-pie -Wl,-Tldscript.x -Wl,-Map -Wl,mapfile.t onexit.c -Wl,--defsym -Wl,_atexit=__atexit -Wl,--section-alignment -Wl,4096 --no-exceptions -o "+Sname+".BIN 2>&1")
        exe(STRIP+' '+Sname+".BIN")
    elif buildway == 'rcl':
        exe('vs6.cmd cl -EP -D_RANDOM='+str(randombytes(4))+" -I../../include metastab.c > ../"+Sname+".c")
        exe('vs6.cmd cl ../'+Sname+".c -Fe../"+Sname+".BIN -ML /Zi /O1 -I. /link /debug /pdb:../image00400000.pdb /fixed:no /incremental:no advapi32.lib user32.lib /map /safeseh:no")
        unlink(Sname+".obj")
    else:
        exe('vs6.cmd cl -EP -D_RANDOM='+str(randombytes(4))+" -D_METAPROC -I../../include metastab.c > ../metaproc1.c")
        exe(PYTHON+' metaproc.py ../metaproc1.c '+hex(randombytes(33))[4:-1]+' > ../'+Sname+".c")
        exe('vs6.cmd cl ../'+Sname+".c -Fe../"+Sname+".BIN -ML /Zi /O1 -I. /link /debug /pdb:../image00400000.pdb /fixed:no /incremental:no advapi32.lib user32.lib /map /safeseh:no")
        unlink(Sname+".obj")

    unlink("../metaproc1.c")

    bS = os.path.basename("../"+Sname)
    S = open("../"+Sname+".BIN","rb").read()
    print "S_size: ", len(S), Sname+".BIN"
    S_size = len(S)
    S_crc  = zlib.crc32(S)
    S = zlib.compress(S,9)
    print "S_size_compressed: ", len(S)
    o = open("../"+Sname+".inl","w+")
    o.write("unsigned char "+bS+"[] = {\n")
    for i in struct.pack("i",S_size):
        o.write("%d,"%ord(i))
    o.write("\n")
    for i in struct.pack("i",S_crc):
        o.write("%d,"%ord(i))
    o.write("\n")
    k = 0
    for i in S:
        o.write("%d,"%ord(i))
        if k and k % 23 == 0:
            o.write("\n")
        k = k + 1
    o.write("\n};\n")
    o.write("unsigned "+bS+"_size = %d;\n"%(len(S)+8) )
    o.close()

for i in range(N):
    Q(Sname+str(i))

S = open("../"+Sname+"NN.inl","w+")
S.write("#define STUBS_COUNT %d\n"%N)

for i in range(N):
    S.write("#include \"%s.inl\"\n"%(os.path.basename(Sname+str(i))))

S.write("struct stubSdata { unsigned char * data; unsigned size; };\n")
S.write("typedef struct stubSdata stubSdata;\n")
S.write("stubSdata stubS[] = {\n");
for i in range(N):
    bS = os.path.basename(Sname+str(i))
    S.write("{ "+bS+", "+bS+"_size },\n" )
S.write("{0,0}};\n");
