
#
#
# Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name
#
#

import sys

name = sys.argv[1]
xormask = ord(sys.argv[2])
text = sys.argv[3]

r = []

s = "0x%02x" % xormask
for i in text:
    r.append( ord(i) ^ xormask )
    xormask = ((xormask << 1) | (xormask >> 7)) & 0x0ff

for i in r:
    s+= (", 0x%02x" % i)
print "static XoredStringA<%d> %s = {{" % (len(text)+1,name)
print s
print "}};"
