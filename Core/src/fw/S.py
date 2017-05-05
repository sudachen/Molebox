
import re, random, os, os.path, time

files = ['fwinstall.c']

def Main():

	xored_strings = []
	for i in xrange(int(random.random() * 3)):
		xored_strings.append(int(random.random()*0xff))
	
	xor_magic = int(time.time())

	for f_name in files:
		f = open(f_name,"r")
		fo = open(f_name+'.xor',"w+")
		for l in f.xreadlines():
			j = l
			while True:
				m = re.search(r'_S\((\d*)\s*,\s*"((?:(\\")|[^"])*)"\)',j)
				if m:
					print m.group()
					lS = m.group(2)
					lSq = lS.replace('\\\\','\\').replace('\\"','"').replace('\\n','\n').replace('\\0','\0')
					for i in xrange(int(random.random()*5+1)): xored_strings.append(int(random.random()*0xff))
					magic = (len(xored_strings) + (len(xored_strings) << 16)) ^ xor_magic
					fo.write(j[0:m.start()])
					fo.write(r'_S(%d,"%s")'%(magic,lS))
					j = j[m.end():]
					S = int(random.random()*0x80) + 5
					xored_strings.append(S)
					for i in lSq:
						Q = ord(i)
						xored_strings.append(S^Q)
						S = ((Q << 4) | (Q >> 4))&0x0ff 
					xored_strings.append(S)
					continue
				else:
					break
			fo.write(j)
		fo.close()
		f.close()
		if os.path.exists( f_name+'.bak' ) : os.unlink(f_name+'.bak')
		os.rename(f_name,f_name+'.bak')
		os.rename(f_name+'.xor',f_name)

	fS = open("fwxor.S","w+")
	fS.write('.section .text$xor, "rx"\n')
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
.global __S_Table
__S_Table:
	mov     4(%esp),%eax
	call    __S_Table@e
__S_Table@e:
	xor     $'''+str(xor_magic)+''',%eax
	and     $65535,%eax
	addl    (%esp),%eax
	add     $4,%esp
	sub     $(__S_Table@e-XORED_TABLE),%eax
	ret
''')

Main()
