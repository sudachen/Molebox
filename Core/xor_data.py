
import re,random,os,rfc822,sys

def xor_part(part,mask):
    def unhex(a):
        f = {
            '0':0,'1':1,'2':2,'3':3,'4':4,'5':5,'6':6,'7':7,'8':8,'9':9,
            'a':10,'A':10,'b':11,'B':11,'c':12,'C':12,
            'd':13,'D':13,'e':14,'E':14,'f':15,'F':15}
        return (f[a[0]]<<4)|(f[a[1]])

    part = map(unhex,part.split('\\x'))
    for i in xrange(len(part)):
        part[i] = int(part[i] ^ mask) & 0x0ff
        mask = (mask >> 1) | int(mask << 31)
    part = ''.join(map(lambda x: '\\x%02x'%x, part))
    return part,mask

def Main(fname):
    f_in_xdata = False
    f_xor_mask = 0
    f = open(fname,"r")
    fo = open(fname+'.xor',"w+")
    for l in f.xreadlines():
        j = l
        if f_in_xdata:
            m = re.search('\/\/END XORED DATA,([0-9]+)\/\/',j)
            if m:
                f_in_xdata = False
            else:
                m = re.search('([^/]*)\"[^\"]*\"[^/]*\/\/([^\"]*)"\\\\x([^\"]*)"(.*)',j)
                if m:
                    lPref = m.group(1)
                    lCmnt = m.group(4)
                    lPart = m.group(3)
                    nPart,f_xor_mask = xor_part(lPart,f_xor_mask)
                    j = lPref + '%-24s'%('"'+nPart+'"') + '// "\\x'+lPart+'"' + lCmnt + '\n'
                    #print j
        else:
            m = re.search('\/\/BEGIN XORED DATA,([0-9]+)\/\/',j)
            if m:
                lS = m.group(1)
                f_xor_mask = 1664525*int(lS) + 1013904223
                f_in_xdata = True
                #print lS
        fo.write(j)
    fo.close()
    f.close()
    os.unlink(fname)
    os.rename(fname+'.xor',fname)

#print sys.argv
Main(sys.argv[1])
