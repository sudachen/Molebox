#!/usr/bin/env python

import os, os.path, sys

print os.path.abspath(os.path.dirname(__file__))
os.chdir(os.path.abspath(os.path.dirname(__file__)))

if sys.platform != 'win32':
  sys.exit(os.system("wine python makeit.py -v -jpro "+' '.join(sys.argv[1:])))

sys.path.append('../py')
from make import *
sys.path.append('..')
import myenv

set_build_config('static')
process_command_line();

vcmarker = ''
if get_jVal('vc6'):
    print 'building vc6 path'
    VISUS = os.environ['VISUS']
    PSDK  = VISUS+'\\PSDK\\'
    VS60  = VISUS+'\\M$drugs\\6\\vc98\\'
    os.putenv('PATH',VS60+'bin'+';'+os.environ['PATH'])
    os.putenv('LIB',PSDK+'lib'+';'+VS60+'lib')
    os.putenv('INCLUDE',PSDK+'include'+';'+VS60+'include'+';'+PSDK+'include\\mfc')
    vcmarker = '-vc6'

if get_jVal('vc10'):
    vcmarker = '-10mt'
    RT_static = True
if get_jVal('vc10md'):
    vcmarker = '-10md'
    RT_static = False

BUILDDIR = os.getenv('BUILDDIR')
if not BUILDDIR: BUILDDIR = os.environ['TEMP']

#libname = '../teggo.classes-%s-%s.lib' % ( get_build_config(), get_build_type() )
#libname = '../teggo.classes-%s-%s.lib' % ( get_build_config(), get_build_type() )

if get_build_config().startswith('export') or get_build_config().startswith('shared') :
    global_flags_set['C_FLAGS'] += ['-D_TEGGODYNAMIC_EXPORT']
    static = False
else:
    global_flags_set['C_FLAGS'] += ['-D_TEGGOSTATIC']
    static = True

if get_build_config().startswith('shared'):
    libext = '.dll'
else:
    libext = '.lib'

tempdir = BUILDDIR+('/teggo.molebox4.classes%s-%s-%s'+libext) % ( vcmarker, get_build_config(), get_build_type() )
print tempdir
debug_suffix = ''
if get_build_type() == 'debug':
    debug_suffix = '-debug'

if get_build_config().startswith('shared'):
    libname = '../classes%s%s' % (vcmarker,get_build_config()[6:]) + debug_suffix + libext
    imp_lib_flags = [ '-implib:../teggo.classes%s-%s%s.lib' % (vcmarker,get_build_config(),debug_suffix) ]
else:
    libname = ('../teggo.classes%s-%s' + debug_suffix + libext) % (vcmarker,get_build_config())

exports_flags = ['-debug','-incremental:no','-nodefaultlib:msvcprt.lib','-nodefaultlib:msvcprtd.lib']

add_global_flags_for_msvc(no_throw=True,static=RT_static,no_debug=False) #(not debug_suffix))
global_flags_set['C_FLAGS'] += ['-nologo','-Z7','-I%VISUS%/dx7sdk/include']

sources = [
    ['./_adler32.cpp',[],['./_adler32.h']],
    ['./_codecop.cpp',[],['./_codecop.inl']],
    ['./_crc32.cpp',[],['./_crc32.h']],
    ['./_specific.cpp',[],['./_specific.inl']],
    ['./collection.cpp',[],['./collection.inl']],
    ['./com_dynamic.cpp',[],['./com_dynamic.inl','./com_generic.h']],
    ['./com_register.cpp',[],['./com_register.inl','./com_generic.h']],
    ['./format.cpp',[],['./format.h']],
    ['./genericio.cpp',[],['./genericio.inl']],
    ['./hinstance.cpp',[],['./hinstance.h']],
    ['./keylist.cpp',[],['./keylist.inl']],
    ['./logger.cpp',[],['./logger.inl']],
    ['./lz77ss.cpp',[],['./lz77ss.inl']],
    ['./lz+.cpp',[],['./lz+.inl']],
    ['./lz+decoder.cpp',[],['./lz+decoder.inl']],
    ['./zlib.cpp',[],['./zlib.inl','./zlib.h']],
    ['./messages.cpp',[],['./messages.inl']],
    ['./newdes.cpp',[],['./newdes.inl']],
    ['./saxparser.cpp',[],['./saxparser.inl','./saxparser.h','./streams_u2.inl']],
    ['./defparser.cpp',[],['./defparser.inl','./saxparser.h','./defparser.h','./xdom.h']],
    ['./streams.cpp',[],['./streams.inl']],
    ['./string.cpp',[],['./string.inl','./string.h']],
    ['./symboltable.cpp',[],['./symboltable.inl']],
    ['./sysutil.cpp',[],['./sysutil.h']],
    ['./threads.cpp',[],['./threads.h']],
    ['./xdom.cpp',[],['./xdom.inl','./xdom.h','./saxparser.h']],
    './newdes_accel.S']

if not get_jVal('vc10md'):
    sources = sources + [
    #'./Z/zl_adler32.c',
    './Z/zl_compress.c',
    #'./Z/zl_crc32.c',
    './Z/zl_deflate.c',
    './Z/zl_infblock.c',
    './Z/zl_infcodes.c',
    './Z/zl_inffast.c',
    './Z/zl_inflate.c',
    './Z/zl_inftrees.c',
    './Z/zl_infutil.c',
    './Z/zl_trees.c',
    './Z/zl_uncompr.c',
    './Z/zutil.c',
    ]

objects = compile_files(sources,tempdir)

if get_build_config().startswith('shared'):
    libs = ['ole32.lib','oleaut32.lib','advapi32.lib','user32.lib','kernel32.lib','shell32.lib']
    link_shared(objects,libs,tempdir,libname,imp_lib_flags + exports_flags)
else:
    link_static(objects,tempdir,libname)
