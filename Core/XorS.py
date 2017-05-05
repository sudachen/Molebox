
#
#
#  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name
#
#

import re, random, os, os.path, rfc822, sys
#os.chdir(os.path.abspath(os.path.dirname(__file__)))

def Main():

    xored_strings = []
    for i in xrange(int(random.random() * 32)):
        xored_strings.append(int(random.random()*0xff))
    xor_magic = int(random.random() * 20070511)

    L = os.listdir('.');
    L = [ i for i in L if i.endswith('.cpp') ]

    for i in L:
        do_rename = False
        f_name = i
        f = open(f_name,"r")
        fo = open(f_name+'.xorS',"w+")
        for l in f.xreadlines():
            j = l
            while True:
                m = re.search(r'_XOr\("((?:(\\")|[^"])*)",[^\)]*\)',j)
                if m:
                    do_rename = True
                    lS = m.group(1)
                    lSq = lS.replace('\\\\','\\').replace('\\"','"').replace('\\n','\n').replace('\\r','\r').replace('\\0','\0')
                    for i in xrange(int(random.random()*5+1)): xored_strings.append(int(random.random()*0xff))
                    magic = (len(xored_strings) + (len(xored_strings) << 16) + xor_magic) ^ xor_magic
                    fo.write(j[0:m.start()])
                    fo.write(r'_XOr("%s",%d,%d)'%(lS,len(lSq)+1,magic))
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
            sys.stderr.write("XORS: "+f_name+"\n");
            os.unlink(f_name)
            os.rename(f_name+'.xorS',f_name)
        else:
            os.unlink(f_name+'.xorS')

    #fS = open("xorS.S","w+")
    
    if len(sys.argv) > 1 :
        fS = open(sys.argv[1],"w+")
    else:
        fS = sys.stdout
    
    fS.write('.data\n')
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
XORED_BUFFER:
.skip 128,0

.text
.global __UnXOrSfree
__UnXOrSfree:
    lea     XORED_BUFFER,%eax
    cmp     %eax,4(%esp)
    je      __UnXOrFree_free_local
    push    4(%esp)
    push    $0
    call    _GetProcessHeap@0
    push    %eax
    call    _HeapFree@12
    ret
__UnXOrFree_free_local:
    decl    -4(%eax)
    ret

.global __UnXOrS
__UnXOrS:
    lea     XORED_BUFFER,%eax
    cmpl    $0,-4(%eax)
    jne     __UnXOr_heap_allocate
    cmpl    $128,8(%esp)
    jg      __UnXOr_heap_allocate
    incl    -4(%eax)
    jmp     __UnXOr_buffer
__UnXOr_heap_allocate:
    push    8(%esp)
    push    $0
    call    _GetProcessHeap@0
    push    %eax
    call    _HeapAlloc@12
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
    lea     XORED_TABLE(%esi),%esi
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