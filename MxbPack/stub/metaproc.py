import sys,os,random,hashlib,binascii,shelve,time

#print os.path.abspath(os.path.dirname(__file__))
os.chdir(os.path.abspath(os.path.dirname(__file__)))

sys.path = [os.path.abspath(os.path.dirname(__file__))] + sys.path
from pycparser import c_parser, c_ast

import ndes

msvcrt_funcs = [
(
    ('free',1),
    ('malloc',1),
    ('realloc',2),
    ('sscanf',3),
    ('sprintf',3),
),
]

advapi_funcs = [
(
    ('RegOpenKeyW',3),
    ('RegQueryValueW',4),
    ('RegCloseKey',1),
    ),
(('RegDeleteKeyW',2),),
(('RegSetValueW',5),),
(('RegCreateKeyExW',9),),
(('RegCreateKeyA',3),),
#(('RegCreateKeyExA',9),),
#(('RegQueryValueA',4),),
(('RegQueryValueExW',6),),
(('RegQueryValueExA',6),),
#(('RegDeleteKeyA',2),),
#(('RegEnumValueA',8),),
(('RegEnumValueW',8),),
(('RegOpenKeyExA',5),),
(('RegOpenKeyExW',5),),
#(('RegOpenKeyA',3),),
(('RegSetValueA',5),),
(('RegSetValueExW',6),),
#(('RegSetValueExA',6),),
]

kernel32_funcs = [
(
    ('CreateFileW',7),
    ('CloseHandle',1),
    ('InitializeCriticalSection',1),
    ('GetModuleHandleA',1),
    ('GetModuleFileNameW',3),
    ('LoadLibraryW',1),
    ('HeapFree',3),
    ('HeapAlloc',3),
    ('HeapCreate',3),
    ('HeapDestroy',1),
    ('TerminateProcess',2)
),
(('CreateFileA',7),),
(('WriteFile',5),),
(('ReadFile',5),),
(('GetFileSize',2),),
(('SetFilePointer',4),),
#(('CreateThread',6),),
(('ExitProcess',1),),
#(('TerminateProcess',2),),
#(('TerminateThread',2),),
(('EnterCriticalSection',1),('LeaveCriticalSection',1)),
(('TlsAlloc',0),('TlsGetValue',1),('TlsSetValue',2),('TlsFree',1)),
#(('CreateFiber',3),('SwitchToFiber',1),('ConvertThreadToFiber',1)),
(('CreateFileMappingW',6),('MapViewOfFile',5),('UnmapViewOfFile',1)),
]



if False:
    user32_funcs = [ (('MessageBoxW',4),)]
else:
    user32_funcs = [ #(('MessageBoxW',4),)]
    (
        ('EnableWindow',2),('ShowWindow',2),
        ('GetParent',1),
        ('UpdateWindow',1),
        ('DefWindowProcW',4),
        ('CallWindowProcW',5),
        ('SetWindowTextW',2),
        ('GetWindowTextLengthW',1),
        ('InvalidateRect',3),
        ('SystemParametersInfoW',4),
        ('CreateWindowExW',12),
        ('DestroyWindow',1),
        ('TrackPopupMenu',7),
        ('AdjustWindowRectEx',4),
        ('UnregisterClassW',2),
        ('RegisterClassW',1),
        ('GetMessageW',4),
        ('PostMessageW',4),
        ('LoadIconW',2),
        ('LoadBitmapW',2),
        ('LoadImageW',6),
        ('EndPaint',2),
        ('BeginPaint',2),
        ('GetUpdateRect',3),
        ('LoadCursorW',2),
        ('MessageBoxW',4),
        ('CreatePopupMenu',0),
        ('DestroyMenu',1),
        ('GetSystemMetrics',1),
        ('GetDC',1),
        ('ReleaseDC',2),
        ('PeekMessageW',5),
        ('TranslateMessage',1),
        ('DispatchMessageW',1),
        ('PostQuitMessage',1),
        ('SendMessageW',4),
        ('MessageBeep',1),
        ('GetDlgItem',2),
        ('GetWindowTextW',3),
        ('GetClassNameW',3),
        ('DestroyCursor',1),
        ('CreateMenu',0),
        ('DefFrameProcW',5),
        ('OpenClipboard',1),
        ('CloseClipboard',0),
    ),
    (('DrawMenuBar',1),('EnableMenuItem',3),('GetSystemMenu',2),),
    (('GetWindowRect',2),('GetClientRect',2),('FillRect',3),),
    (('GetWindowLongW',2),('SetWindowLongW',3),),
    (('SetWindowPos',7),('SetFocus',1),('GetFocus',0),),
    (('GetCapture',0),('SetCursor',1),('SetCapture',1),('ReleaseCapture',0),('SetCursorPos',2),),
    (('GetScrollInfo',3),('SetScrollInfo',4),('ScrollWindow',5)),
    (('WindowFromPoint',2),('GetCursorPos',1),('ScreenToClient',2),('ClientToScreen',2),),
    (('SetParent',2),),
    (('RedrawWindow',4),),
    (('MoveWindow',6),),
    (('DeferWindowPos',8),('EndDeferWindowPos',1),('BeginDeferWindowPos',1),),
    (('IsWindowVisible',1),),
    (('IsWindowEnabled',1),),
    (('GetWindow',2),),
    (('GetMessageTime',0),),
    (('GetMenuItemInfoW',4),('GetMenuItemCount',1),),
    (('GetActiveWindow',0),),
    (('CallNextHookEx',4),('UnhookWindowsHookEx',1),('SetWindowsHookExW',4),),
    (('RegisterHotKey',4),('UnregisterHotKey',2),),
    (('PtInRect',3),),
    (('IsWindow',1),),
    (('IsDialogMessageW',2),),
    (('GetSysColor',1),),
    (('GetUpdateRgn',3),),
    (('InflateRect',3),),
    (('CreateDialogParamW',5),),
    (('GetKeyState',1),),
    (('BringWindowToTop',1),),
    (('IsZoomed',1),),
    (('IsIconic',1),),
    (('GetDesktopWindow',0),),
    (('SetWindowRgn',3),),
    (('FlashWindow',2),),
    (('CreateDialogIndirectParamW',5),),
    (('GetForegroundWindow',0),('SetForegroundWindow',1),),
    (('IsClipboardFormatAvailable',1),),
    (('SetClipboardData',2),),
    (('EnumClipboardFormats',1),),
    (('GetClipboardData',1),),
    (('EmptyClipboard',0),),
    (('HideCaret',1),('ShowCaret',1),),
    (('DrawTextW',5),),
    (('DrawFocusRect',2),),
    (('CopyRect',2),),
    (('OffsetRect',3),),
    (('KillTimer',2),('SetTimer',4),),
    (('GetIconInfo',2),('DestroyIcon',1),('CreateIconIndirect',1),('DrawIcon',4),),
    (('SetMenu',2),('SetMenuItemInfoW',4),('ModifyMenuW',5),('RemoveMenu',3),('InsertMenuW',5),('GetMenuState',3),),
    (('GetAsyncKeyState',1),),
    (('VkKeyScanW',1),),
    (('EnumDisplaySettingsW',3),('ChangeDisplaySettingsW',2),),
    (('ShowCursor',1),),
    (('DrawStateW',10),('DrawEdge',4),),
    (('GetMenuStringW',5),),
    (('DefMDIChildProcW',4),),
    (('TranslateMDISysAccel',2),),
    (('AppendMenuW',4),),
    (('GetClassInfoW',3),),
    (('WaitForInputIdle',2),),
    (('GetClipboardFormatNameW',3),),
    (('RegisterClipboardFormatW',1),),
    (('ChildWindowFromPoint',3),),
    (('UnionRect',3),),
    (('GetWindowDC',1),),
    (('EnumWindows',2),),
    (('ExitWindowsEx',2),),
    (('GetWindowThreadProcessId',2),),
    (('TranslateAcceleratorW',3),),
    (('CreateAcceleratorTableW',2),('DestroyAcceleratorTable',1),('LoadAcceleratorsW',2),),
    (('ValidateRect',2),),
    (('DrawIconEx',9),),
    (('DrawFrameControl',4),),
    (('MsgWaitForMultipleObjects',5),),
    (('PostThreadMessageW',4),),
    (('GetMessagePos',0),),
    (('MapWindowPoints',4),),
    (('LoadCursorFromFileW',1),),
    (('CheckMenuRadioItem',5),),
    (('CheckMenuItem',3),),
    (('InsertMenuItemW',4),),
    (('GetSubMenu',2),),
    ]

def _ExX(i):
    if len(i) > 2:
        return (i[0],i[1])
    else:
        return i

class Extfunc:
    def __init__(self,S):
        self.S = S
    def PrintFunc(self):
        print self.S

def randombytes(n):
    bits = os.urandom(n)
    q = 0L
    while bits:
        q = (q<<8) | ord(bits[0])
        bits = bits[1:]
    return q;


def DeclareExternFuncs(mt,funcs):
    for q,i in extern_funcs:
        if q is not None:
            for x in i:
                #sys.stderr.write( "x : %s\n" % repr(x) )
                f,a = _ExX(x)
                print 'extern int '+q+' '+f+'('+','.join(['int a'+str(j) for j in range(a)])+');'
                print '#ifdef __WAY1'
                print 'int ('+q+' *(fk_'+f+'()))('+','.join(['int ' for j in range(a)])+');'
                print '#else'
                print 'int ('+q+' *(qfk_'+f+'()))('+','.join(['int ' for j in range(a)])+');'
                print '#define fk_'+f+' qfk_'+f
                print '#endif'

                S = '\n#ifdef __WAY1\n' \
                + 'int ('+q+' *(fk_'+f+'()))('+','.join(['int ' for j in range(a)])+'){' \
                + '  static double _f = 0;' \
                + '  if ( !_f ) _f = (double)((unsigned long)&'+f+')*10;' \
                + '  return (void*)(unsigned long)(_f*0.333333333333331*(double)__LINE__/0x'+str(randombytes(3))+'); }' \
                + '\n#elif defined __TRIAL\n' \
                + 'int ('+q+' *(fk_'+f+'()))('+','.join(['int ' for j in range(a)])+'){' \
                + '  void *_f = 0;' \
                + '  if ( !_f ) _f = &'+f+';' \
                + '  return _f; }' \
                + '\n#else\n' \
                + 'int ('+q+' *(qfk_'+f+'()))('+','.join(['int ' for j in range(a)])+'){' \
                + '  return encode_ptr('+f+'); }' \
                + '\n#endif\n'
                funcs.append(Extfunc(S))
        else:
            for x in i:
                #sys.stderr.write( "x : %s\n" % repr(x) )
                f,a = _ExX(x)
                print '#define fk_'+f+'() '+f

funcs_exclude = ['chrcmp','seql','adler32','do_cipher_block','decompress']
#funcs_exclude = ['chrcmp','seql'] #,'unxor','Wunxor']

XOR_PASSWD = os.urandom(15)
print "static unsigned char _XORKEY[15] = {"+','.join(map(lambda a:'0x%02x'%ord(a), XOR_PASSWD))+"};"
class Mutagen:
    def Init(self,S):
        self.S = S
        self.random = [0]*4
        self.random[0] = ord(S[0]) + (ord(S[1]) << 8) + (ord(S[2]) << 16) + (ord(S[3]) << 24)
        self.random[1] = ord(S[4]) + (ord(S[5]) << 8) + (ord(S[6]) << 16) + (ord(S[7]) << 24)
        self.random[2] = ord(S[8]) + (ord(S[9]) << 8) + (ord(S[10]) << 16) + (ord(S[11]) << 24)
        self.random[3] = ord(S[12]) + (ord(S[13]) << 8) + (ord(S[14]) << 16) + (ord(S[15]) << 24)
        self.rstate = list(self.random)
        self.unique_vals = [0]*8
        def _B(x):
            for i in range(8):
                yield ((ord(x) >> i)&1) != 0
        self.seq =  reduce( lambda x,y: list(x) + list(y), map(_B, S[16:]) )
        self._GenerateUniqueVals()
        self.bitN = 0
        self.unique_strings = []
    @classmethod
    def _GenerateS(cls):
        return os.urandom(32) # 16 + 16, 16/4 => 4, 16*8 => 128
    def __init__(self,S=None):
        if not S:
            S = Mutagen._GenerateS()
        self.Init(S)
    def Bit(self,n):
        return self.seq[n%128]
    def ResetBits(self):
        self.bitN = 0
    def SeqBit(self):
        self.bitN += 1
        return self.Bit(self.bitN-1)
    def Random(self,n):
        return self.random[n%4]
    def _GenerateUniqueVals(self,):
        for i in range(len(self.unique_vals)):
            self.unique_vals[i] = self.GenerateUniqueExclution(3)
    def RandomUniqueVal(self):
        return self.unique_vals[self.Random(0,len(self.unique_vals))]
    def UniqueVal(self,n):
        return self.unique_vals[n%len(self.unique_vals)]
    def _Random(self,n):
        n = n % 4
        self.rstate[n] = int((1664525 * self.rstate[n] + 1013904223) % 0x7fffffff);
        return self.rstate[n]
    def GenerateUniqueExclution(self,n=0):
        while True:
            q = self._Random(n%4)
            q = q if q >= 0 else -q
            if q not in self.unique_vals:
                return q
    def UniqueString(self):
        while True:
            q = self._Random(3)
            if q not in self.unique_strings:
                self.unique_strings.append(q)
                return self._Md5string(q)
    def _Md5string(self,q):
        m = hashlib.md5()
        m.update(str(q))
        return '_'+binascii.b2a_hex(m.digest())
    def Random(self,min,max):
        return int(( self._Random(0) % max ) + min)
    def RandomList(self,l):
        ll = len(l)
        for i in range(ll):
            a = self.Random(0,ll)
            b = self.Random(0,ll)
            if a != b:
                q = l[a]
                l[a] = l[b]
                l[b] = q

class Polymorph:

    MAX_GLOBAL_VARS = 30
    MIN_GLOBAL_VARS = 10
    MAX_FAKE_FUNCS  = 70
    MIN_FAKE_FUNCS  = 30
    MAX_LINES_PER_FAKE = 5
    MIN_LINES_PER_FAKE = 1
    MAX_LINES_INFAKE = 3
    MIN_LINES_INFAKE = 1

    def __init__(self,m):
        self.mutagen = m
        self._GenerateGlobalVars()
        self._GenerateRandomFuncs()
        self.g_predef_funcs = [
            #('',),
            ]

    def _GenerateGlobalVars(self):
        self.g_vars = ['']*self.mutagen.Random(self.MIN_GLOBAL_VARS,self.MAX_GLOBAL_VARS)
        for i in range(len(self.g_vars)):
            self.g_vars[i] = self.mutagen.UniqueString()

    def _GenerateRandomFuncs(self):
        self.g_funcs = [ \
            (self.mutagen.UniqueString(), self.mutagen.Random(0,7)) \
                for i in range(self.mutagen.Random(self.MIN_FAKE_FUNCS,self.MAX_FAKE_FUNCS)) ]

    def PrintRandomFunc(self,no):
        n,a = self.g_funcs[no]
        args = [ 'a'+str(i) for i in range(a) ]
        xtable = dict([(q,'int') for q in args])
        class X:
            def __init__(self,table,xtable=None):
                self.table = table
                self.xtable = xtable
        state = {}
        decls = []
        lines = []
        self.GenLocals(None,decls,state,X({},X(xtable)),0)
        for i in range(self.mutagen.Random(self.MIN_LINES_INFAKE,self.MAX_LINES_INFAKE)):
            lines.append(self.GenCmplxExpr(state,True))
        print 'int '+n+'('+','.join(['int '+q for q in args])+'){'
        for i in decls: print i+';'
        for i in lines: print i+';'
        print 'return '+self.GenRawExpr(state) + ';'
        print '}'

    def GetFakes(self):
        class X:
            def __init__(self,no,ph):
                self.ph = ph
                self.no = no
            def PrintFunc(self):
                self.ph.PrintRandomFunc(self.no)
        return [ X(i,self) for i in range(len(self.g_funcs)) ]

    def PrintGlobals(self):
        for i in self.g_vars:
            print 'int '+i+' = '+str(self.mutagen.GenerateUniqueExclution())+';'
        for n,a in self.g_funcs:
            print 'int '+n+'('+','.join(['int']*a)+');'
        for n,a in self.g_predef_funcs:
            print 'extern void __stdcall '+n+'('+','.join(['int']*a)+');'

    def GenLocals(self,in_l,out_l,state,t,o):
        n = []
        if in_l is not None:
            nl = len(in_l)
        else:
            nl = self.mutagen.Random(1,6)
        if nl:
            n = ['']*self.mutagen.Random(1,nl);
        state['step'] = o
        vars = {}
        def traverse(t,n=1):
            for i,j in t.table.items():
                ii = i
                if "[" in i: continue
                if i.startswith('('):
                    ii = i[1:].strip(')').strip().split(' ')[-1].strip('*')
                vars[ii] = True
                #if "*f" in i: sys.stderr.write("$ "+i+' -> '+ii)
                if t.xtable and n:
                    traverse(t.xtable,++n)
        if t.xtable:
            traverse(t.xtable)
        state['vars'] = map( lambda x :x[0], vars.items() )
        for i in range(len(n)):
            n[i] = 'int ___local_'+str(i)+'_'+str(o)+' = '+self.GenRawExpr(state)
        traverse(t,0)
        state['vars'] = map( lambda x: x[0], vars.items() )

        if not in_l: in_l = []
        self.RandomAdd(in_l,n,out_l)

        state['locals'] = ['___local_'+str(i)+'_'+str(o) for i in range(len(n))]
        for i in state['locals']:
            vars[i] = True
            t.table[i] = 'int'

        state['vars'] = map( lambda x: x[0], vars.items() )
        #sys.stderr.write(repr(vars)+'\n')

    def GenStatements(self,in_l,out_l,state,t,o):
        #sys.stderr.write(repr(state)+'\n')
        n = []
        ln = len(in_l)
        if len(in_l) > 4:
            n = ['']*self.mutagen.Random(1,(ln*3)/5);
            for i in range(len(n)):
                n[i] = self.GenCmplxExpr(state)
        self.RandomAdd(in_l,n,out_l)

    def RandomConstantOrName(self,state):
        q = self.mutagen.Random(0,1000)
        v = state['vars']
        l = len(v)
        if q < 200 or not l:
            return str(self.mutagen.GenerateUniqueExclution())
        if q < 400:
            return '((int)'+self.g_vars[self.mutagen.Random(0,len(self.g_vars))]+')'
        else:
            return '((int)'+v[self.mutagen.Random(0,l)]+')'

    def RandomExprOrName(self,state):
        q = self.mutagen.Random(0,1000)
        v = state['vars']
        l = len(v)
        if q < 200 or not l:
            #return str(self.mutagen.GenerateUniqueExclution())
            return self.GenRawExpr(state)
        if q < 400:
            return '((int)'+self.g_vars[self.mutagen.Random(0,len(self.g_vars))]+')'
        else:
            return '((int)'+v[self.mutagen.Random(0,l)]+')'

    def GenRawExpr(self,state):
        ops = ['+','-','*','&','&&','||']
        e = ''
        n = self.mutagen.Random(2,5)
        for i in range(n):
            if not i:
              e = '((int)'+self.RandomConstantOrName(state)+')'
            else:
              op = ops[self.mutagen.Random(0,len(ops))]
              e = e + op + '((int)'+self.RandomConstantOrName(state)+')'
        return e

    def GenCmplxExpr(self,state,w=False):
        loc = state['locals']
        vars = state['vars']
        q = self.mutagen.Random(0,1000)
        uv = self.g_vars[self.mutagen.Random(0,len(self.g_vars))]
        if q < 200:
            e = self.GenRawExpr(state)
        else:
          if not w:
              e = '('+uv+'=='+str(self.mutagen.RandomUniqueVal())+')?'\
                +self.GenRandomCall(state)\
                +':'+self.GenRawExpr(state)
          else:
              if q < 300:
                  e = self.GenRawExpr(state)
              if q < 600:
                  e = self.GenRandomCall(state)
              else:
                  e = self.GenRandomCall(state)
                  #e = self.GenExternCall(state)
        if len(loc):
            e = loc[self.mutagen.Random(0,len(loc))] + '=' + e
        return e

    def GenRandomCall(self,state):
        f,a = self.g_funcs[self.mutagen.Random(0,len(self.g_funcs))]
        args = ','.join([ self.RandomConstantOrName(state) for i in range(a) ])
        return f+'('+args+')'

    def GenExternCall(self,state):
        q = self.mutagen.Random(0,len(externs)*2)
        x = externs[q%len(externs)]
        if len(x) == 2 :
            f, a = x
            args = ','.join([ self.RandomExprOrName(state) for i in range(a) ])
            S = f+'()('+args+')'
        else:
            f, a, Opt = x
            args = []
            for j in Opt:
                if j is not None : args.append(str(j))
                else: args.append( self.RandomExprOrName(state) )
            args = ','.join(args)
            S = f+'()('+ args +')'
        S = 'fk_' + S
        #sys.stderr.write("%s\n" %S)
        return S

    def RandomAdd(self,i1,i2,o):
        while i1 or i2:
            q = self.mutagen.SeqBit()
            if i1:
                if q or not i2:
                    o.append(i1[0])
                    i1 = i1[1:]
                    continue
            if i2:
                if not q or not i1:
                    o.append(i2[0])
                    i2 = i2[1:]
                    continue

class Printer1(c_ast.NodeVisitor):
    def __init__(self):
        self.structs_n_unions ={}

    def TypeName(self,t):
        if type(t) == c_ast.PtrDecl:
            return self.TypeName(t.type) + ' * '
        elif type(t) == c_ast.TypeDecl:
            return self.TypeName(t.type)
        elif type(t) == c_ast.Struct:
            return 'struct '+t.name
        elif type(t) == c_ast.IdentifierType:
            return ' '.join(t.names)
        elif type(t) == c_ast.Union:
            return 'union '+t.name
        else:
            raise Exception('invalid node '+ t.__class__.__name__)

    def visit_ID(self,t):
        return t.name
    def visit_UnaryOp(self,t):
        if t.op.startswith('p'):
            return '('+self.visit(t.expr)+')'+t.op[1:]
        if t.op == 'sizeof':
            return 'sizeof('+self.visit(t.expr)+')'
        return t.op+'('+self.visit(t.expr)+')'
    def visit_FuncDef(self,t):
        return self.visit(t.decl)
    def visit_Enumerator(self,t):
        val = '='+self.visit(t.value) if t.value is not None else ''
        return t.name + val
    def visit_EnumeratorList(self,t):
        return [self.visit(i) for i in t.enumerators]
    def visit_Enum(self,t):
        n = t.name if t.name is not None else ''
        if t.values is not None:
            return 'enum '+n+'{'+','.join(self.visit(t.values))+'}'
        else:
            return 'enum '+n
    def visit_Typedef(self,t):
        #t.show()
        tt,nn,xx = self.visit(t.type)
        nn = self.TypeName(t.type)
        k = ''
        if nn.startswith('struct') or nn.startswith('union'):
            q = nn.strip()
            while q[-1] == '*': q = q[:-1].strip()
            if q not in self.structs_n_unions:
                k = tt.strip()
                while k[-1] == '*': k = k[:-1].strip()
                k = k + ';'
                self.structs_n_unions[q] = True
        return k + 'typedef '+nn+' '+t.name
    def visit_FileAST(self,t):
        for i in t.ext:
            print self.visit(i),';'
    def visit_Typename(self,t):
        tt,nn,xx = self.visit(t.type)
        return tt+' '+nn+xx
    def visit_ParamList(self,t):
        return '('+','.join([self.visit(i) for i in t.params])+')'
    def visit_FuncDecl(self,t):
        tt,nn,xx = self.visit(t.type)
        args = t.args if t.args is not None else []
        #print args, tt, nn, xx
        xx = self.visit(t.args) if args else '()'
        #if t.calldecl == 'stdcall':
        #    nn = '(__stdcall '+nn+')'
        return (tt,nn,xx)
    def visit_BinaryOp(self,t):
        return '('+self.visit(t.left) +')' + t.op + '('+self.visit(t.right)+')'
    def visit_Union(self,t):
        n = t.name if t.name is not None else ''
        if t.decls:
            tt = 'union '+n+'{' + ';'.join([self.visit(i) for i in t.decls]) + ';' + '}'
        else:
            tt = 'union '+n
        return tt;
    def visit_Struct(self,t):
        n = t.name if t.name is not None else ''
        #t.show()
        if t.decls:
            tt = 'struct '+n+'{' + ';'.join([self.visit(i) for i in t.decls]) + ';' + '}'
        else:
            tt = 'struct '+n
        return tt;
    def visit_Constant(self,t):
        return t.value
    def is_funcPtr(self,t):
        if type(t.type) == c_ast.PtrDecl:
            return self.is_funcPtr(t.type)
        elif type(t.type) == c_ast.FuncDecl:
            return True
        else:
            return False
    def visit_ArrayDecl(self,t):
        tt,n,x = self.visit(t.type)
        if self.is_funcPtr(t.type):
            i = n.find(')')
            if i > 0: n1 = n[:i]; n2 = n[i:]
            else: n1 = n; n2 = ""
            if t.dim:
                return (tt, n1+ '[' + self.visit(t.dim) + ']'+n2, x)
            else:
                return (tt, n1+ '[]'+n2, x)
        else:
            if t.dim:
                return (tt,n, x + '[' + self.visit(t.dim) + ']')
            else:
                return (tt,n, x + '[]')
    def visit_IdentifierType(self,t):
        return ' '.join(t.names).strip()
    def visit_TypeDecl(self,t):
        tt = self.visit(t.type)
        nn = t.declname if t.declname is not None else ''
        return (tt,nn,'')
    def visit_PtrDecl(self,t,c=1):
        #t.show()
        q = ''
        if t.quals:
            q = ' '.join(t.quals)+' '
        if type(t.type) == c_ast.PtrDecl:
            return self.visit_PtrDecl(t.type,c+1)
        elif type(t.type) == c_ast.FuncDecl:
            tt,n,x = self.visit(t.type)
            return (tt,'('+q+'*'*c+n+')',x)
        else:
            tt,n,x = self.visit(t.type)            
            return (tt + q +'*'*c, n , x)
    def visit_Decl(self,t):
        if type(t.type) == c_ast.Enum :
            return self.visit(t.type)
        else:
            Sq = self.visit(t.type)
            #sys.stderr.write("%s\n"%repr(Sq))
            tt,nn,xx = Sq
            q = ''
            if t.storage:
                q = q+' '.join(t.storage)+' '

            if type(t.type) == c_ast.TypeDecl or type(t.type) == c_ast.ArrayDecl:
                if t.type.attributes:
                    for i in t.type.attributes:
                        if type(i) == c_ast.Declspec:
                            q = '__declspec('+i.name+') '+q

            ini = ''
            bS  = ''
            if t.bitsize:
                bS = ':'+self.visit(t.bitsize)
            if t.init:
                ini = self.visit(t.init)
                if type(t.init) == c_ast.ExprList:
                    ini = '{' + ini + '}'
            if ini: ini = '='+ini
            xxx = ''
            while type(t.type) == c_ast.PtrDecl :
                t = t.type
            #if type(t.type) == c_ast.ArrayDecl:
            #    t.show()
            if type(t.type) == c_ast.TypeDecl or type(t.type) == c_ast.ArrayDecl:
                if t.type.attributes:
                    for i in t.type.attributes:
                        if type(i) != c_ast.Declspec:
                            expr = '' if not i.expr else '('+self.visit(i.expr)+')'
                            xxx = xxx + ' __attribute__(('+i.name+expr+'))'
            return q + tt+' '+nn+xx+xxx+bS+ini
    def visit_Cast(self,t):
        return '(('+self.visit(t.to_type)+')('+self.visit(t.expr)+'))'
    def visit_ExprList(self,t):
        return ','.join([self.visit(i) for i in t.exprs])
    def visit_FuncCall(self,t):
        def xor(T):
            #k = ord(os.urandom(1)[0])
            #L = ['\\x%02x'%k]
            #if T.startswith('L'): T = T[1:]
            #for i in T.strip('"'):
            #    k = (k + 1) % 256
            #    L.append('\\x%02x'%(ord(i)^k))
            #k = (k + 1) % 256
            #return '"'+''.join(L) + ('\\x%02x'%k) + '"'
            if T.startswith('L'): T = T[1:]
            T = T.strip('"')
            #sys.stderr.write(T)
            #sys.stderr.write('\n')
            T = T + '\0'
            T = '"'+''.join(map(lambda x: '\\x%02x'%ord(x), ndes.encrypt_S(XOR_PASSWD,T)))+'"'
            #sys.stderr.write(T)
            #sys.stderr.write('\n')
            return T
        def unxor(T):
            #L = []
            #k = ord(T[0])
            #for i in T[1:]:
            #    k = (k + 1) % 256
            #    i = ord(i) ^ k
            #    if i:
            #        L.append(chr(i))
            #    else:
            #        L.append('\\0')
            #return ''.join(L)
            return ""
        S = self.visit(t.name)
        if S == '__unxor' or S == '__Wunxor':
            B,T = t.args.exprs
            B = self.visit(B)
            T = self.visit(T)
            T = xor(T)
            if S == '__unxor':
                return 'unxor('+B+','+T+') /*' + unxor(eval(T)) + '*/'
            else:
                return 'Wunxor('+B+','+T+') /*' + unxor(eval(T)) + '*/'
        else:
            return S+'('+(self.visit(t.args) if t.args else '')+')'
    def visit_StructRef(self,t):
        return self.visit(t.name)+t.type+self.visit(t.field)
    def visit_ArrayRef(self,t):
        return self.visit(t.name)+'['+self.visit(t.subscript)+']'
    def visit_Compound(self,t):
        decls = ''
        stmts = ''
        if t.decls:
            decls = ';'.join([self.visit(i) for i in t.decls])
        if t.stmts:
            stmts = ';'.join([self.visit(i) for i in t.stmts])
        if decls and stmts: decls = decls + ';'
        return '({'+decls+stmts+';})'
    def visit_Assembler(self,t):
        #t.show()
        volatile = '' if not t.volatile else ' volatile '
        params = ''
        if t.params:
            for i in t.params:
                params = params + ':'
                if i:
                    params = params + ','.join([ a+' ('+self.visit(e)+')' for a,e in i])
        return '__asm__'+volatile+'('+t.code+' '+params+')'
    def visit_Quote(self,t):
        return '\n'+t.code.strip('"').replace("\\\"","\"")+'\n'
    def visit_Assignment(self,t):
        return '('+self.visit(t.lvalue)+')' + t.op + '('+self.visit(t.rvalue)+')'
    def visit_TernaryOp(self,t):
        return '('+self.visit(t.cond)+ ')?(' + self.visit(t.iftrue) + '):(' + self.visit(t.iffalse) + ')'
    def generic_visit(self, t):
        raise Exception('invalid node '+ t.__class__.__name__)

functions = {}

class FargsTable(c_ast.NodeVisitor):
    def __init__(self,xtable=None):
        self.xtable = xtable
        self.table = {}
    def visit_Decl(self,t):
        tt,nn,_ = Printer1().visit(t.type)
        if tt in ['FUNCS','USW']:
            #sys.stderr.write('skip '+tt+' '+nn+'\n');
            pass
        else:
            return (nn,tt)
    def visit_ParamList(self,t):
        if t.params:
            for i in t.params:
                nn,tt = self.visit(i)
                self.table[nn] = tt
                #print nn,tt
    def visit_FuncDecl(self,t):
        if t.args: self.visit(t.args)
    def generic_visit(self, t):
        raise Exception('invalid node '+ t.__class__.__name__)

class CodeTranslator(c_ast.NodeVisitor):
    def __init__(self,table,o,ph):
        self.t = table
        self.lines = []
        self.o = o
        self.ph = ph
    def visit_Decl(self,t):
        ini = ''
        if type(t.type) == c_ast.Enum:
            return Printer1().visit(t.type)
        else:
            #t.show()
            tt,nn,xx = Printer1().visit(t.type)
            if tt in ['FUNCS','USW']:
                #sys.stderr.write('skip '+tt+' '+nn+'\n');
                pass
            else:
                if self.t:
                    self.t.table[nn] = tt;
            return Printer1().visit(t)
    def visit_Compound(self,t):
        state = {}
        l = []
        if t.decls:
            for i in t.decls:
                l.append('  '*self.o + self.visit(i))

        if self.t:
            self.ph.GenLocals(l,self.lines,state,self.t,self.o)
        else:
            self.lines.extend(l)

        if t.stmts:
            l = []
            for i in t.stmts:
                S = self.visit_comp(i)
                l.append('  '*self.o + S)
            if l and self.t:
                self.ph.GenStatements(l,self.lines,state,self.t,self.o)
            else:
                self.lines.extend(l)
    def visit_Return(self,t):
        if t.expr:
            return 'return ' + Printer1().visit(t.expr)
        else:
            return 'return '
    def visit_Goto(self,t):
        return 'goto ' + t.name
    def visit_comp(self,t):
        if type(t) == c_ast.Compound:
            xt = None
            if self.t:
                xt = FargsTable(self.t)
            c = CodeTranslator(xt,self.o+1,self.ph)
            c.visit(t)
            #t.show()
            #print c.lines
            S = '{\n' + ';\n'.join(c.lines) + ';\n}'
        else:
            S = self.visit(t)
        return S
    def visit_While(self,t):
        #t.show()
        S = 'while ('+Printer1().visit(t.cond)+')'
        if t.stmt : S = S + self.visit_comp(t.stmt)
        return S
    def visit_For(self,t):
        #t.show()
        I = Printer1().visit(t.init) if t.init else ''
        C = Printer1().visit(t.cond) if t.cond else ''
        N = Printer1().visit(t.next) if t.next else ''
        S = 'for ('+I+';'+C+';'+N+')'
        return S + self.visit_comp(t.stmt)
    def visit_DoWhile(self,t):
        #t.show()
        S = 'while ('+Printer1().visit(t.cond)+')'
        SS = 'do ' + self.visit_comp(t.stmt)
        if not t.stmt or type(t.stmt) is not c_ast.Compound: SS = SS+';'
        return SS + S
    def visit_If(self,t):
        S = 'if ('+Printer1().visit(t.cond)+')'
        if t.iftrue : S = S + self.visit_comp(t.iftrue)
        if not t.iftrue or type(t.iftrue) is not c_ast.Compound: S = S+';'
        if t.iffalse : S = S +'\nelse ' + self.visit_comp(t.iffalse)
        return S
    def visit_Switch(self,t):
        S = 'switch ('+Printer1().visit(t.cond)+')\n'
        if t.stmt:
            S = S + self.visit_comp(t.stmt)
        return S
    def visit_FuncCall(self,t):
        return Printer1().visit(t)
    def visit_Assignment(self,t):
        return Printer1().visit(t)
    def visit_Assembler(self,t):
        return Printer1().visit(t)
    def visit_Quote(self,t):
        return Printer1().visit(t)
    def visit_ExprList(self,t):
        return Printer1().visit(t)
    def visit_UnaryOp(self,t):
        return Printer1().visit(t)
    def visit_Default(self,t):
        S = self.visit_comp(t.stmt) if t.stmt else ''
        return 'default:' + S
    def visit_Case(self,t):
        S = self.visit_comp(t.stmt) if t.stmt else ''
        return 'case '+Printer1().visit(t.expr)+':' + S
    def visit_Break(self,t):
        return 'break'
    def visit_Continue(self,t):
        return 'continue'
    def visit_Label(self,t):
        S = self.visit_comp(t.stmt) if t.stmt else ''
        return t.name + ':' + S
    def generic_visit(self, t):
        raise Exception('invalid node '+ t.__class__.__name__)

class Function():
    def __init__(self,name,decl,table,code):
        #decl.show()
        self.name = name
        self.rettype = Printer1().visit(decl.type)[0]
        self.args = [ Printer1().visit(i) for i in decl.type.args.params ] if decl.type.args else []
        self.table = table
        #sys.stderr.write(self.name + repr(table.table) + '\n')
        self.code = code
    def Printarg(self,a):
        tt,nn,_ = Printer1().visit(a)
        return tt + ' ' + nn
    def PrintDecl(self):
        print self.rettype + ' ' + self.name + '(' + ','.join(self.args)+')'
    def PrintCode(self):
        print '{'
        for i in self.code:
            print i +';'
        print '}'
    def PrintFunc(self):
        self.PrintDecl()
        self.PrintCode()

class Printer2(c_ast.NodeVisitor):

    def __init__(self,ph):
        self.ph = ph

    def visit_FuncDef(self,t):
        #t.show()
        #print 'function', t.decl.name
        args = FargsTable()
        args.visit(t.decl.type)
        if t.decl.name not in funcs_exclude:
            table = FargsTable(args)
        else:
            table = None
        code = CodeTranslator(table,1,self.ph)
        code.visit(t.body)
        #print code.lines
        functions[t.decl.name] = Function(t.decl.name,t.decl,table,code.lines)
    def visit_Decl(self,t):
        if type(t.type) == c_ast.TypeDecl or type(t.type) == c_ast.PtrDecl:
            pass

    def visit_Typedef(self,t):
        pass
    def visit_FileAST(self,t):
        self.table = FargsTable()
        for i in t.ext:
            self.visit(i)
    def visit_Quote(self,t):
        pass
    def generic_visit(self, t):
        raise Exception('invalid node '+ t.__class__.__name__)

#raise 0
def main(fname,unique,F):
    global extern_funcs, externs
    global advapi_funcs,kernel32_funcs,user32_funcs,msvcrt_funcs

    parser = c_parser.CParser()
    text = open(fname,'r').read()
    L = []
    for i in text:
        if i == '\r': L.append('')
        else: L.append(i)
    text = ''.join(L)

    ast = parser.parse(text, filename=fname,debuglevel=0)
    #ast.show()

    p1 = Printer1()
    p1.visit(ast)

    if unique:
        unique = binascii.a2b_hex(unique)
    else:
        while True:
            unique= os.urandom(32);
            if F and not F.has_key(unique):
                F[unique] = time.time()
                break
            else:
                break

    mt = Mutagen(unique)

    def EfLrandomize(L,mt):
        R = list(L[0])
        for i in L[1:]:
            q = mt.SeqBit()
            if q:
                R.extend(i)
        return R

    mt.ResetBits()
    advapi_funcs = EfLrandomize(advapi_funcs,mt)
    kernel32_funcs = EfLrandomize(kernel32_funcs,mt)
    user32_funcs = EfLrandomize(user32_funcs,mt)
    msvcrt_funcs = EfLrandomize(msvcrt_funcs,mt)
    mt.ResetBits()

    extern_funcs = [
    (None,msvcrt_funcs),
    ('__stdcall',advapi_funcs),
    ('__stdcall',kernel32_funcs),
    ('__stdcall',user32_funcs),
    ]


    externs = reduce( lambda x,y: x + y, map( lambda z: z[1], extern_funcs) )

    ph = Polymorph(mt)

    p2 = Printer2(ph)
    p2.visit(ast)

    funcs = [ j for i,j in functions.items()]
    fakes = ph.GetFakes()

    ph.PrintGlobals()
    DeclareExternFuncs(mt,funcs);

    mt.RandomList(funcs)
    mt.RandomList(fakes)

    while funcs or fakes:
        q = mt.SeqBit()
        f = None
        if (q and funcs) or (not q and not fakes):
            f = funcs[0]
            funcs = funcs[1:]
        elif (q and not funcs) or (not q and fakes):
            f = fakes[0]
            fakes = fakes[1:]
        f.PrintFunc()

if len(sys.argv) < 2:
    print 'usage metaproc.py original.c unique'
    sys.exit(1)

argv = sys.argv
fname = argv[1]
argv = argv[2:]
unique = None
F = None
if argv:
    unique = argv[0]
    if unique == '@':
        #F = unique[1:]
        unique = None
        F = shelve.open("dbase/unique",'c')
main(fname,unique,F)
