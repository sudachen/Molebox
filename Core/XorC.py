
#
#
#  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name
#
#

import re, random, os, os.path, rfc822, sys
#os.chdir(os.path.abspath(os.path.dirname(__file__)))

def Main():

    xored_strings = []
    for i in xrange(int(random.random() * 3)):
        xored_strings.append(int(random.random()*0xff))
    xor_magic = int(random.random() * 20070511)

    L = os.listdir('.');
    L = [ i for i in L if i.endswith('.cpp') or i.endswith('.c')]

    for i in L:
        do_rename = False
        f_name = i
        f = open(f_name,"r")
        fo = open(f_name+'.xorC',"w+")
        for l in f.xreadlines():
            j = l
            while True:
                """
                m = re.search(r'_XOr\("((?:(\\")|[^"])*)",(\d*),(\d*),([^\)]*)\)',j)
                if m:
                    lS = m.group(1)
                    lSq = lS.replace('\\\\','\\').replace('\\"','"').replace('\\n','\n').replace('\\0','\0')
                    for i in xrange(int(random.random()*5+1)): xored_strings.append(int(random.random()*0xff))
                    magic = (len(xored_strings) + (len(xored_strings) << 16) + xor_magic) ^ xor_magic
                    fo.write(j[0:m.start()])
                    fo.write(r'_XOr("%s",%d,%d,%s)'%(lS,len(lSq)+1,magic,m.group(5)))
                    j = j[m.end():]
                    S = int(random.random()*0x80) + 5
                    xored_strings.append(S)
                    for i in lSq:
                        xored_strings.append(S^ord(i))
                        S = (S >> 1 | S << 7 ) & 0xff
                    xored_strings.append(S)
                    continue
                m = re.search(r'_uXOr\("((?:(\\")|[^"])*)",(\d*),(\d*),([^\)]*)\)',j)
                if m:
                    lS = m.group(1)
                    lSq = lS.replace('\\\\','\\').replace('\\"','"').replace('\\n','\n').replace('\\0','\0')
                    for i in xrange(int(random.random()*5+1)): xored_strings.append(int(random.random()*0xff))
                    magic = (len(xored_strings) + (len(xored_strings) << 16) + xor_magic) ^ xor_magic
                    fo.write(j[0:m.start()])
                    fo.write(r'_uXOr("%s",%d,%d,%s)'%(lS,(len(lSq)+1)*2,magic,m.group(5)))
                    j = j[m.end():]
                    S = int(random.random()*0x80) + 5
                    xored_strings.append(S)
                    for i in lSq:
                        xored_strings.append(S^ord(i))
                        S = (S >> 1 | S << 7 ) & 0xff
                        xored_strings.append(S)
                        S = (S >> 1 | S << 7 ) & 0xff
                    xored_strings.append(S)
                    S = (S >> 1 | S << 7 ) & 0xff
                    xored_strings.append(S)
                    continue
                """
                m = re.search(r'_XOrX\("((?:(\\")|[^"])*)",([0-9]*)[^\)]*\)',j)
                if m:
                    do_rename = True
                    lS = m.group(1)
                    lSq = lS.replace('\\\\','\\').replace('\\"','"')
                    mask = int(m.group(3))
                    fo.write(j[0:m.start()])
                    xored = ''.join( r'\x%02x' % (ord(i) ^ mask) for i in lSq)
                    fo.write(r'_XOrX("%s",%d,"%s")'%(lS,mask,xored))
                    j = j[m.end():]
                    continue
                m = re.search(r'_iXOr\("((?:(\\")|[^"])*)",(\d*),(\d*)\)',j)
                if m:
                    do_rename = True
                    lS = m.group(1)
                    lSq = lS.replace('\\\\','\\').replace('\\"','"').replace('\\n','\n').replace('\\0','\0')
                    for i in xrange(int(random.random()*5+1)): xored_strings.append(int(random.random()*0xff))
                    magic = (len(xored_strings) + (len(xored_strings) << 16) + xor_magic) ^ xor_magic
                    fo.write(j[0:m.start()])
                    fo.write(r'_iXOr("%s",%d,%d)'%(lS,len(lSq)+1,magic))
                    j = j[m.end():]
                    S = int(random.random()*0x80) + 5
                    xored_strings.append(S)
                    for i in lSq:
                        xored_strings.append(S^ord(i))
                        S = (S >> 1 | S << 7 ) & 0xff
                    xored_strings.append(S)
                    continue
                break
            fo.write(j)
        fo.close()
        f.close()
        if do_rename:
            sys.stderr.write("XORC: "+f_name+"\n");
            os.unlink(f_name)
            os.rename(f_name+'.xorC',f_name)
        else:
            os.unlink(f_name+'.xorC')

    if len(sys.argv) > 1 :
        fS = open(sys.argv[1],"w+")
    else:
        fS = sys.stdout

    fS.write('.section .XORS\n')
    fS.write('XORED_TABLE:')

    for i in xrange(len(xored_strings)):
        if i % 16 == 0:
            fS.write('\n.byte ')
        else:
            fS.write(',')

        fS.write('0x%02x'%xored_strings[i])

    fS.write('\n')
    fS.write(\
'''
.align 4
.long 0

.global __UnXOr
__UnXOr:
    mov     12(%esp),%eax

__UnXOr_buffer:
    push    %esi
    push    %edi
    push    %ecx

    mov     %eax,%edi
    movl    16(%esp),%esi
    movl    $'''+str(xor_magic)+''',%eax
    xor     %eax,%esi
    sub     %eax,%esi
    and     $(0x0ffff),%esi

    call    __UnXOr_TABLE
__UnXOr_TABLE:
    pop     %eax
    sub     $(__UnXOr_TABLE-XORED_TABLE),%eax
    add     %eax,%esi
    movl    20(%esp),%ecx
    movl    (%esi),%eax
    incl    %esi
    push    %edi

__UnXOr_loop:
    movb    (%esi),%ah
    xorb    %al,%ah
    movb    %ah,(%edi)
    incl    %esi
    ror     $1,%al
    incl    %edi
    decl    %ecx
    jne     __UnXOr_loop

    pop     %eax
    pop     %ecx
    pop     %edi
    pop     %esi
    ret

''')

Main()
