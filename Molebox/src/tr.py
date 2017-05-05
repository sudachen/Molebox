
#
#
#  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name
#
#

import re, random, os, os.path, rfc822, sys

def Main():

    force = False
    last_number  = 1;
    strings = {}

    for f_name in os.listdir("."):

	if not f_name.endswith(".cpp"): 
            continue
		
        do_rename = False
        print f_name
        f = open(f_name,"r")
        fo = open(f_name+'.tr~',"w+")
        for l in f.xreadlines():
            j = l
            while True:
                m = re.search(r'Tr\((\d*),"((?:(\\")|[^"])*)"\)',j)
                if m:
                    do_rename = True
                    lS = m.group(2)
                    lN = last_number
                    last_number += 1
                    strings[lN] = unicode(lS)
                    fo.write(j[0:m.start()])
                    fo.write('Tr(%d,"%s")'%(lN,lS))
                    j = j[m.end():]
                    continue
                break
            fo.write(j)
        fo.close()
        f.close()
        if do_rename:
            os.unlink(f_name)
            os.rename(f_name+'.tr~',f_name)
        else:
            os.unlink(f_name+'.tr~')

    fm = open('tr.inl',"w+")
    fm.write("const size_t _TR_COUNT_ = "+str(last_number)+";\n")
    fm.write("const char* _TR_["+str(last_number)+"] = {\n")
    for l,j in strings.items():
        fm.write("\"%d:%s\",\n"%(l,j.encode('utf8')))
    fm.write("};\n")
    fm.close()

if __name__ == '__main__':
    Main()
