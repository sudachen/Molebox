
/*

Copyright (C) 2010, Alexey Sudachen alexey@sudachen.name

*/

#include "nt.h"
#include "stbinfo.h"

#ifndef _ROTOR
#ifndef _METAPROC
#define _ROTOR "\372\233\017\241\005\204\240\165\255\370\055\334\175\034\236\021\024\031\015\032\263\375\010\376\150\244\213\223\266\002\312\162\215\235\147\202\250\221\076\033\210\042\350\161\023\256\325\053\373\260\133\342\315\124\173\270\027\307\305\035\201\115\164\234\220\317\326\001\313\067\152\070\211\151\261\061\362\113\353\145\363\131\230\332\243\171\206\041\335\057\337\012\323\306\130\110\304\103\227\226\125\146\052\020\252\371\046\367\040\327\006\333\336\075\013\253\276\036\011\214\054\132\231\330\262\351\022\045\060\225\320\144\360\205\025\143\331\341\340\141\232\277\174\203\345\102\267\200\377\016\224\065\163\254\321\217\172\136\154\117\004\051\366\356\273\106\137\275\311\166\077\156\007\355\310\003\374\322\000\314\044\134\062\037\157\064\324\123\176\121\347\142\122\160\153\251\274\026\245\354\112\116\155\135\107\302\105\140\300\111\126\265\247\246\071\167\043\177\063\237\272\303\212\352\364\170\050\072\100\242\316\114\120\222\357\343\344\361\271\127\365\104\014\030\047\301\264\074\207\101\073\066\056\346\216\257"
#define _ROTOR_SID 1
#else
__quote "#include <rotor.h>"
#endif
#endif

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef short int16_t;
typedef unsigned int uint32_t;
typedef int int32_t;
#ifndef _METAPROC
#if _MSC_VER >= 1400 || defined __GNUC__
typedef unsigned long long uint64_t;
typedef long long int64_t;
#else
typedef unsigned __int64 uint64_t;
typedef __int64 int64_t;
#endif
#else
	__quote "typedef unsigned __int64 uint64_t;"
	__quote "typedef __int64 int64_t;"
#endif
typedef uint32_t size_t;

//#include "pe.h"

typedef struct FILE FILE;
FILE* _wfopen(const wchar_t* filename, const wchar_t* opts);
void* malloc(size_t size);
void free(void*);
int memcmp(const void* _Buf1, const void* _Buf2, size_t _Size);
void* memcpy(void* dst, const void* src, size_t _Size);
void _assert(const char* _Message, const char* _File, unsigned _Line);
int puts(const char*);
#ifndef _METAPROC
void* (__stdcall GetModuleHandleA)(void*);
int   (__stdcall GetModuleFileNameW)(void*, wchar_t*, int);
void* (__stdcall LoadLibraryW)(wchar_t*);
void  (__stdcall HeapFree)(void*, uint32_t, void*);
void* (__stdcall HeapAlloc)(void*, uint32_t, uint32_t);
void* (__stdcall HeapCreate)(uint32_t, uint32_t, uint32_t);
void* (__stdcall HeapDestroy)(void*);
void  (__stdcall TerminateProcess)(uint32_t, uint32_t);
#endif

extern void* _crtheap;

#ifdef _DEBUG
#define assert(_Expression) (void)( (!!(_Expression)) || (_assert(#_Expression, __FILE__, __LINE__), 0) )
#else
#define assert(_Expression)
#endif

void abort();

#define __attribute__(x)

#define CLOCKS_PER_SEC 1000
long clock(void);

static unsigned char jb[256] = {0};
void *spurge;

enum { BLOCK_BYTES = 8 };
enum { KEY_BYTES   = 15 };
enum { UNRAV_BYTES = 60 };
enum { UNRAV_BYTES_1 = 68 };

typedef struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	int tm_yday;
	int tm_isdst;
} struct_tm;

typedef unsigned long time_t;
struct_tm*localtime(time_t *_Time);
time_t mktime(struct_tm *_Tm);

#define _XOr(x,l,S,p) __unxor(p,x)
#define _XOr1(x,l,S,p) __unxor(p,x)
#define _uXOr(x,l,S,p) __Wunxor(p,L##x)

#define TRAP_MARKER_1 0xafd19301
#define TRAP_MARKER_2 0x53125102
#define TRAP_MARKER_3 0xabe0c103
#define TRAP_MARKER_4 0x563f1904
#define TRAP_MARKER_5 0x88765405
#define TRAP_MARKER_6 0xcf278a06
#define TRAP_MARKER_7 0xbb865307
#define TRAP_MARKER_8 0x480a2908
#define TRAP_MARKER_9 0xa3a2ff09
#define TRAP_MARKER_a 0x3ff4560a
#define SSS(x) #x
#define SSSS(x) SSS(x)

#define __attribute__(x)

char *_TIB()
{
	char *Q;
#if defined _METAPROC
	__quote "__asm mov eax, fs:[0x18] __asm mov Q, eax";
#else
	__asm mov eax, fs:[0x18] __asm mov Q, eax;
#endif
	return Q;
}

//#define _ASM_INT_3_(x) __asm mov ecx,x __asm int 3
#define _ASM_INT_3_(x) abort(x)
#define _PEB()   (*((void**)(_TIB()+48)))

unsigned _UNIQUE()
{
	uint8_t *Q = _TIB();
	unsigned j = *(unsigned*)(Q+32);
	unsigned k = *(unsigned*)(Q+36);
	return ((k << 4) + j)&65535;
}

void __unxor(void *b,void *s) { strcpy((char*)b,(char*)s); }
void __Wunxor(void *b,void *s) { wcscpy((wchar_t*)b,(wchar_t*)s); }

uint8_t xor_byte_0(uint8_t a, uint8_t b);
uint8_t xor_byte_1(uint8_t a, uint8_t b);

void unxor(void *b, void *s);
void Wunxor(void *b, void *s);

#define _DLO() ( ((void***)_PEB())[3][3] )
#define _BASE() ( ((void**)_PEB())[2] )
#define _HEAP() ( ((void**)_PEB())[6] )
#define _PROCENV() ( ((void***)_PEB())[4][18] )
#define _STB_INFO() ( (STB_INFO*) ((void***)_PEB())[2][9] )
#define _STB_INFO_SET(p) ( ((STB_INFO***)_PEB())[2][9] = (STB_INFO*)(p) )

#define PEB_LDR(x) ( *(unsigned int*)((char*)x + 0xc) )
#define PEB_LDR_DLLODER(x) ( (char*)PEB_LDR(x) + 0xc )
#define FLINK(x) ( ((void**)x)[0] )
#define BLINK(x) ( ((void**)x)[1] )
#define TIB_PEB(x) (  *(unsigned char*)((char*)x + 0x30) )
#define DLO_BASE(x) ( ((unsigned int*)x)[24/4] )
#define DLO_NAME(x) ( ((unsigned int*)x)[40/4] )
#define DLO_NLEN(x) ( ((unsigned int*)x)[36/4] & 0x0ff )

int f_return_0() { return 0; }
char* encode_ptr(void* p);
void* decode_ptr(char* S);

uint8_t left_xor_byte(uint8_t a, uint8_t b)
{
	return a & ~b;
}

uint8_t right_xor_byte(uint8_t a, uint8_t b)
{
	return ~a & b;
}

uint8_t xor_byte_0(uint8_t a, uint8_t b)
{
	uint8_t c = right_xor_byte(a, b);
	c |= left_xor_byte(a, b);
	return c;
}

uint8_t xor_byte_1(uint8_t a, uint8_t b)
{
	uint8_t c = left_xor_byte(a, b);
	c |= right_xor_byte(a, b);
	return c;
}

uint32_t left_xor_dword(uint32_t a, uint32_t b)
{
	uint32_t aa = a;
	uint32_t bb = ~b;
	aa &= bb;
	return aa;
}

uint32_t right_xor_dword(uint32_t a, uint32_t b)
{
	uint32_t aa = ~a;
	uint32_t bb = b;
	aa &= bb;
	return aa;
}

uint32_t xor_dword(uint32_t a, uint32_t b)
{
	uint32_t c;
	c = left_xor_dword(a, b);
	c |= right_xor_dword(a, b);
	return c;
}

void decode_rotor_f(void (**decode_rotor)(), uint8_t* e, uint8_t* d, uint8_t* dE, uint32_t seed)
{
	uint32_t i;
	seed = seed * 1664525 + 1013904223;
	i = seed % (dE - d);
	*d = e[i];
	memcpy(e + i, e + i + 1, (dE - d) - i);
	++d;
	decode_rotor[d != dE](decode_rotor, e, d, dE, seed);
}

void switch_heap()
{
	_crtheap = HeapCreate(0x40000, 0, 0);
}

const uint8_t* rotor()
{
	static uint8_t* decoded = 0;
	if (!decoded)
	{
		void (*a[2])() = { switch_heap, decode_rotor_f };
		uint8_t r[256];
		memcpy(r, _ROTOR, 256);
		decoded = malloc(256);
		a[1](a, r, decoded, decoded + 256, _ROTOR_SID);
		assert(memcmp(decoded, rotor_S, 256) == 0);
	}
	return decoded;
}

void setup_decipher_key(uint8_t* k, const uint8_t* key, int i)
{
	*(k + 2) = key[(i + 2) % 15];
	*(k + 0) = key[(i + 0) % 15];
	*(k + 3) = key[(i + 3) % 15];
	*(k + 1) = key[(i + 1) % 15];

	i = (i + 3) % 15;
	i = (i + 9) % 15;

	if (i == 12)
	{
		return;
	}

	k += 4;

	*(k++) = key[i++];
	*(k++) = key[i++];
	*(k++) = key[i];
	i = (i + 9) % 15;

	setup_decipher_key(k, key, i);
}

void setup_decipher(uint8_t* k, const uint8_t* key)
{
	setup_decipher_key(k, key, 11);
}

void do_cipher_inner_2_f(void(**f)(), uint8_t* k, uint8_t* b, int count)
{
	--count;
	b[3 - count + 4] = xor_byte_1(b[3 - count + 4], rotor()[xor_byte_0(b[3 - count], *(k++))]);
	f[!!count](f, k, b, count);
}

void do_cipher_inner_f(void(**f)(), uint8_t* k, uint8_t* b, int count)
{
	--count;
	b[7] = xor_byte_0(b[7], rotor()[xor_byte_0(b[3], *(k + 3))]);
	b[4] = xor_byte_0(b[4], rotor()[xor_byte_1(b[0], *(k + 0))]);
	b[6] = xor_byte_1(b[6], rotor()[xor_byte_1(b[2], *(k + 2))]);
	b[5] = xor_byte_1(b[5], rotor()[xor_byte_0(b[1], *(k + 1))]);
	k += 4;
	b[1] = xor_byte_1(b[1], rotor()[xor_byte_0(b[4], *(k++))]);
	b[2] = xor_byte_0(b[2], rotor()[xor_byte_1(b[4], b[5])]);
	b[3] = xor_byte_0(b[3], rotor()[xor_byte_0(b[6], *(k++))]);
	b[0] = xor_byte_1(b[0], rotor()[xor_byte_1(b[7], *(k++))]);
	f[2 + !!count](f, k, b, count ? count : 4);
}

void do_cipher_8(uint8_t* k, uint8_t* b)
{
	void (*f[4])() =
	{
		(void*)f_return_0,
		(void*)do_cipher_inner_2_f,
		(void*)do_cipher_inner_2_f,
		(void*)do_cipher_inner_f,
	};

	f[2 + (b && k)](f, k, b, 8);
}

void do_cipher_CBCO_xor(uint8_t* q, uint8_t* w, uint8_t* data, int i)
{
	*data = xor_byte_1(*data, *q);
	*q++ = *w++;
	if (--i > 0)
		do_cipher_CBCO_xor(q, w, ++data, i);
}

void do_cipher_CBCO(uint8_t* k, uint8_t* data, int count)
{
	uint8_t w[8];
	uint8_t q[8] = {0};
	int i;

	for (i = 0; i < count; ++i)
	{
		memcpy(w, data + 8 * i, 8);
		do_cipher_8(k, data + 8 * i);
		do_cipher_CBCO_xor(q, w, data + 8 * i, 8);
	}
}

char *Bf_malloc()
{
	return malloc(256+_RANDOM%15);
}

#ifdef __TRIAL

char *encode_ptr(void *ptr)
{
	//return encode_ptr_S(ptr,encode_ptr_gen_S());
	return (char*)xor_dword((uint32_t)ptr,(_RANDOM*1664525 + 1013904223));
}

void *decode_ptr(char *S)
{
	//return decode_ptr_S(S);
	return (void*)xor_dword((uint32_t)S,(_RANDOM*1664525 + 1013904223));
}

#else

time_t base_number(int o)
{
	double number;
	time_t n;
	time_t t = (time(0) + _RANDOM/1759)&0x7fffff00 + _RANDOM%101;
	struct_tm *tm;
	tm = localtime(&t);
	tm->tm_hour = 0;
	tm->tm_min  = 0;
	tm->tm_sec  = 0;
	number = ((double)mktime(tm) + _RANDOM%(60*60*5))/11353;
	n = (time_t)( (number*11353 + ((char*)&o)[_RANDOM%11+8]) + ((clock()/CLOCKS_PER_SEC)&0x0ff) +0.5 );
	return n;
}

void encode_ptr_Q(int *jb, void **ptr)
{
	int i;
	char *S;
	char b[33];
	struct tm *tm;
	double dbl = (unsigned long)*ptr;
	unsigned long Q;
	time_t t = base_number(_RANDOM/11353);
	tm = localtime(&t);
	_XOr("%d%m%Y",8,0,b);
	S = malloc(196+_RANDOM%157);
	strftime(S,_RANDOM/311+_RANDOM/79+33,b,tm);
	Q = (uint32_t)(_RANDOM/1303)*(uint32_t)S*(uint32_t)strlen(S);
	dbl = dbl * Q;
	_XOr("%.0f",4,0,b);
	sprintf(S,b,dbl);
	*ptr = S-1;
	longjmp(jb,1);
}

char *encode_ptr(void *ptr)
{
	int jb[32];
	volatile char *p = ptr;
	int q = 0;	
	if ( !(q = _setjmp(jb)) )
	{
		encode_ptr_Q(jb,(void**)&p);
		return 0;
	}
	return (char*)p+q;
}

void decode_ptr_Q(int *jb, void **p)
{
	int i;
	double dbl;
	unsigned long Q;
	char b[33];
	void *ptr;
	char *S = *p;
	struct tm *tm;
	time_t t = base_number(_RANDOM/11321);
	_XOr("%lf",3,0,b);
	sscanf(S,b,&dbl);
	_XOr("%d%m%Y",8,0,b);
	tm = localtime(&t);
	strftime(S,_RANDOM/51+_RANDOM/13+33,b,tm);
	Q = (uint32_t)(_RANDOM/1303)*(uint32_t)S*(uint32_t)strlen(S);
	free(S);
	ptr = (void*)((unsigned long)((dbl/Q)+0.5));
	*p = (char*)ptr-1;
	longjmp(jb,1);	
}

void *decode_ptr(char *ptr)
{
	int jb[32];
	volatile char *p = ptr;
	int q = 0;
	if ( !(q = _setjmp(jb)) )
	{
		decode_ptr_Q(jb,(void**)&p);
		return 0;
	}
	return (char*)p+q;
}

#endif

int chrcmp(unsigned register val,uint8_t *p)
{
	if (!val) return 1;
	if ( *p == (uint8_t)val || *p == (uint8_t)(val>>8) )
		return chrcmp(val>>16,p + 2);
	return 0;
}

void *INDLLS(void *q)
{
	wchar_t *p = *(wchar_t**)((uint8_t*)q+40);
	wchar_t *s = p;
	//q = p;
	while ( *p++ ) { if (p[-1] == '\\') s = p; }
	return s;
}

int ISNTDLL(void *q)
{
	uint8_t *p = (uint8_t*)INDLLS(q);
	return chrcmp(0x54744e6e,p) && chrcmp(0x4c6c4464,p+4);
}

int ISK32DLL(void *q)
{
	uint8_t *p = (uint8_t*)INDLLS(q);
	return chrcmp(0x45654b6b,p) && chrcmp(0x6e4e5272,p+4)
		&& chrcmp(0x4c6c4565,p+8) && chrcmp(0x32323333,p+12);
}

typedef union USW{
	ANSI_STRING S;
	UNICODE_STRING W;
} USW;

enum { SW_MAXLEN = 64 };
enum { MAX_STB_BUFFER = sizeof(STB_INFO2) };

void Relocate(CORE0_HEADER *eh)
{
	unsigned char *base = (unsigned char*)eh;
	unsigned offs = (unsigned)eh - eh->image_base;
	unsigned char* reloc  = (unsigned char*)eh+eh->relocs;
	unsigned char* relocE = reloc+eh->relocs_size;
	unsigned char* relocX = reloc;

	while ( reloc < relocE )
	{
		unsigned bsize;
		unsigned char const *e;
		int rva = *(int*)reloc; reloc += 4;
		bsize = *(unsigned*)reloc-8; reloc += 4;
		e = reloc+bsize;
		while ( reloc < e )
		{
			switch ( *(unsigned short*)reloc >> 12 )
			{
			default:
				_ASM_INT_3_(TRAP_MARKER_2);
			case 3:
				{
					int fixup = *(unsigned short*)reloc & 0x0fff;
					*(unsigned*)(base+rva+fixup) += offs;
				}
			case 0:
				;
			}
			reloc+=2;
		}
	}
}

unsigned unique_mod(unsigned mod)
{
	return (_UNIQUE()*mod)/((1U<<16)-1);
}

void *find_ntdll(int T)
{
	void *handle = 0;
	void *l = _DLO();
	void *S = FLINK(l);

	while ( S != l )
	{
		if ( T == 0 && ISNTDLL(S) )
		{
			handle = (void*)DLO_BASE(S);
			break;
		}
		else if ( T == 1 && ISK32DLL(S) )
		{
			handle = (void*)DLO_BASE(S);
			break;
		}
		S = FLINK(S);
	}

	return handle;
}

int seql(char *a, char *b)
{
	while ( *a && *b && *a == *b ) ++a, ++b;
	return *a == *b;
}

void *find_func(void *handle,char *name,int _hint)
{
	int i = _hint, fndx;
	IMAGE_NT_HEADERS *nth = NT_HEADERS(handle);
	IMAGE_DATA_DIRECTORY expo = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	IMAGE_EXPORT_DIRECTORY *edir = (IMAGE_EXPORT_DIRECTORY*)((char*)handle+expo.VirtualAddress);
	if ( seql(name,(char*)handle+((unsigned int*)((char*)handle+edir->AddressOfNames))[i]) )
		goto hinted;
	else
		for( i = 0; i < edir->NumberOfNames; ++i )
		{
			if ( !seql(name,(char*)handle+((unsigned int*)((char*)handle+edir->AddressOfNames))[i]) )
				continue;
hinted:
			fndx = ((unsigned short*)((char*)handle+edir->AddressOfNameOrdinals))[i];
			return
				((char*)handle +
				((unsigned int*)((char*)handle+edir->AddressOfFunctions))[fndx]);
		}
		return 0;
}

wchar_t *find_module_name(int* isnested)
{
	void *l = _DLO();
	void *S = l;//FLINK(l);
	wchar_t *pS = (wchar_t*)_PROCENV();
	wchar_t *p  = pS;
	wchar_t b[9];

	_uXOr("XOJUMAN=",18,26476716,b);
	while (*p)
	{
		wchar_t *pS = p;
		while (*p++);
		if ( !memcmp(pS,b,8*2) )
		{
			if ( isnested )
				*isnested = 1;
			return pS+8;
		}
	}
	do
	{
		if ( DLO_BASE(S) == (unsigned)_BASE() )
			return (wchar_t*)DLO_NAME(S);
		S = FLINK(S);
	}
	while ( S != l );

	return 0;
}

enum {
	LZSS_MIN_LEN = 2,
	LZSS_MAX_LEN = 15,
	LZSS_TABLE_MOD = 4095, /*4095, */
	LZSS_TABLE_LEN = LZSS_TABLE_MOD+1,
};

int decompress(uint8_t *in_b, int in_b_len, char *out_b, int out_b_len)
{
	int in_i = 0;
	int out_i = 0;
	while ( in_i < in_b_len && out_i < out_b_len )
	{
		if ( in_b[in_i] == 0x80 )
		{// one char
			out_b[out_i++] = in_b[++in_i];
			++in_i;
		}
		else if ( !in_b[in_i] )
		{// several chars
			int l = (int)in_b[++in_i]+1;
			++in_i;
			while ( l-- )
			{
				out_b[out_i++] = in_b[in_i++];
			}
		}
		else
		{// code
			unsigned short code = (short)in_b[in_i]|((short)in_b[in_i+1] << 8);
			int l = code & 0x0f;
			int off = code >> 4;
			memcpy(out_b+out_i,out_b+out_i-off-LZSS_MAX_LEN,l);
			out_i += l;
			in_i += 2;
		}
	}

	return out_i;
}

#define _ADLER32_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define _ADLER32_DO2(buf,i)  _ADLER32_DO1(buf,i); _ADLER32_DO1(buf,i+1);
#define _ADLER32_DO4(buf,i)  _ADLER32_DO2(buf,i); _ADLER32_DO2(buf,i+2);
#define _ADLER32_DO8(buf,i)  _ADLER32_DO4(buf,i); _ADLER32_DO4(buf,i+4);
#define _ADLER32_DO16(buf)   _ADLER32_DO8(buf,0); _ADLER32_DO8(buf,8);

void printf8(uint8_t *buf)
{
	printf("(%08x) %02x %02x %02x %02x %02x %02x %02x %02x\n",
		   buf,
		   buf[0],buf[1],buf[2],buf[3],
		   buf[4],buf[5],buf[6],buf[7]);
}

CORE0_HEADER *load_embedded(USW *SW, FILE *modfile, uint8_t *keydat, STB_INFO *stb)
{
	unsigned flags, foo, size, l;
	uint8_t *buf, *unpacked;//, *base;
	CORE0_HEADER *eh;

	size = stb->core.count;
	fseek(modfile,stb->core.offset,0);
	buf  = (uint8_t*)malloc(size);
	fread(buf,size,1,modfile);

	do_cipher_CBCO(keydat,buf,size/8);
	l = *(unsigned*)buf;
	unpacked = malloc(l+64*1024);
	unpacked = (uint8_t*)( (((uint32_t)unpacked+1023)&~1023) + unique_mod(63)*1024 );
	l = decompress(buf+4,size-4,unpacked,l);
	eh = (CORE0_HEADER*)unpacked;
	Relocate(eh);

	if (unpacked != buf) free(buf);
	return eh;
}

void *CHAIN1();
void *load_c1(void *ntdll_handle, void **xdll,void **a)
{
	static USW SW = {0};
	static char SW_Buffer[SW_MAXLEN] = {0};

	*xdll = find_ntdll(1);
	*a = &SW;

	return encode_ptr(CHAIN1);
}

void OEPJmp(void *fx,void *stb_info, void *kern32, void *ntdll)
{
	uint8_t *b = (uint8_t*)malloc(16);
	b[25] = 0xff;
	b[32] = 0xff;
	*(void**)(b+1)  = _BASE();    // push base
	b[5] = 0x68;
	*(void**)(b+6)  = stb_info;// push stb_info
	b[20] = 0xb8;
	b[10] = 0x68;
	*(void**)(b+11) = kern32;  // push kern32
	b[0] = 0x68;
	*(void**)(b+16) = ntdll;   // push ntdll
	*(void**)(b+21) = fx;      // move fx,%eax
	b[15] = 0x68;
	b[26] = 0xd0;              // call %eax
	*(void**)(b+28) = spurge;  // mov  $spurge,esp
	b[33] = 0xe0;              // jmp %eax
	b[27] = 0xbc;
#ifdef _METAPROC
	__quote "__asm mov eax,b __asm jmp eax";
#else
	__asm mov eax,b __asm jmp eax;
#endif
}

void *CHAIN1(USW *SW, void* ntdll, void* kern32)
{
	int i,foo,bar,isnested = 0;
	unsigned sign[2];
	void *c1_handle = 0;
	void *c1_activate = 0;
	STB_INFO *stb_info = 0;
	void *(__stdcall *fx)(void*,void*,void*,void*) = 0;
	uint8_t keydat[68];
	FILE* modfile;
	wchar_t *modname; 
	
	modname = find_module_name(&isnested);
	modfile = _wfopen(modname,L"rb");

	if ( 1 )
	{
		unsigned mask;
		IMAGE_NT_HEADERS const *nth = NT_HEADERS(_BASE());
		IMAGE_SECTION_HEADER const *S = (IMAGE_SECTION_HEADER*)((char*)nth + (24 + nth->FileHeader.SizeOfOptionalHeader));
		unsigned fa = (nth->OptionalHeader.FileAlignment-1);
		S+=nth->FileHeader.NumberOfSections-1;
		while ( !S->PointerToRawData ) --S;
		i = (S->PointerToRawData+S->SizeOfRawData+fa)&~fa;

		if ( fseek( modfile, i, 0 ) < 0 || fread(sign,8,1,modfile) < 0  )
			_ASM_INT_3_(TRAP_MARKER_5);
		if ( ((sign[1] ^ STB_SIGN_SECOND) ^ sign[0]) != STB_SIGN_FIRST )
			_ASM_INT_3_(TRAP_MARKER_6);
	}

	foo = MAX_STB_BUFFER;//0x7fffffff;
	//stb_info = _STB_INFO_SET(malloc(MAX_STB_BUFFER));
	stb_info = malloc(MAX_STB_BUFFER);
	fread(stb_info,foo,1,modfile);

	setup_decipher(keydat,stb_info->xcrypt_key);
	do_cipher_CBCO(keydat,(uint8_t*)stb_info,(sizeof(STB_INFO)-16)/8);
	do_cipher_CBCO(keydat,(uint8_t*)(stb_info+1),(MAX_STB_BUFFER-sizeof(STB_INFO))/8);

	if ( isnested ) stb_info->flags |= STBF_EMBEDDED_LOADING|STBF_CHILD;
	((STB_INFO2*)stb_info)->modname = modname;

	foo = 0;

	// starting core
	if ( stb_info->flags & STBF_EXTERNAL_CORE )
	{
		_uXOr("cor1dbg.dll",24,20381967,SW->W.Buffer); SW->W.Length = 24;
		if ( !(c1_handle = LoadLibraryW(SW->W.Buffer)) )
			TerminateProcess(-1,-1);

		_XOr("_Inity@16",10,47907307,SW->S.Buffer);
		fx = find_func(c1_handle,SW->S.Buffer,0);
	}
	else
	{
		CORE0_HEADER *c0 = load_embedded(SW,modfile,keydat,stb_info);
		*(void**)&fx = (char*)c0+c0->inity;
	}

	if ( fx )
		OEPJmp(fx,stb_info,kern32,ntdll);

	TerminateProcess(-1,-1);
	return 0;
}

#ifdef _METAPROC

void init_xored_keydat(unsigned char *keydat)
{
	if ( !*(unsigned int*)keydat )
	{
		setup_decipher(keydat,(uint8_t*)_XORKEY);
	}
}

void decrypt_xored_string(char *o, char *s, int wc)
{
	static unsigned char keydat[68] = {0};
	unsigned char q[8];
	int j;

	init_xored_keydat(keydat);

	for (j=0;;)
	{
		int i;
		memcpy(q,s,8);
		s += 8;
		do_cipher_8(keydat,q);
		for ( i=0; i < 8; ++i,++j )
		{
			o[j] = q[i];
			if ( wc ) { o[j+1] = 0; ++j; }
			if ( !q[i] )
			{
				return;
			}
		}
	}
}

void unxor(void *b, void *s)
{
	decrypt_xored_string(b,s,0);
}

void Wunxor(void *b, void *s)
{
	decrypt_xored_string(b,s,1);
}

#endif

#if defined _MSC_VER
//__quote "#pragma data_seg(\".bss\")"
//#pragma bss_seg(".bss$")
int fake_data[8000];
//__quote "#pragma data_seg(push,\".xtls\")"
//  int aaaa[] = {1};
//  int _threaded_1[512];
//__quote "#pragma data_seg(pop)"
//#pragma data_seg(".tls$")
__declspec(thread) int _threaded_1[512] = {0,};
//#pragma data_seg(".data")
#endif

void *query_load_c1()
{
	return encode_ptr(load_c1);
}

void *query_find_ntdll()
{
	return encode_ptr(find_ntdll);
}

int main(int argv,char **argc)
{
	int i;
	void *f;
	void *ntdll;
	void *xdll  = 0;
	void *a;
	spurge = &argv;
	f = query_find_ntdll();
	f = decode_ptr(f);
	ntdll = ((void*(*)())f)(0);
	f = query_load_c1();
	f = decode_ptr(f);
	f = ((void*(*)())f)(ntdll,&xdll,&a);
	f = decode_ptr(f);
	((void(*)())f)(a,ntdll,xdll);
	memset(fake_data,(uint8_t)f,1);
	return 0;
}

#if !defined _MSC_VER
DWORD _tls_fake[512] __attribute__ ((section (".tls$1"))) = {0};
#endif
#if 1
DWORD _tls_index = 0;
DWORD _tls_start __attribute__ ((section (".tls"))) = 0;
DWORD _tls_end   __attribute__ ((section (".tls$ZZZ"))) = 0;
void * __xl_a __attribute__ ((section (".CRT$XLA"))) = 0;
void * __xl_z __attribute__ ((section (".CRT$XLZ"))) = 0;
IMAGE_TLS_DIRECTORY _tls_used __attribute__ ((section (".rdata"))) = {&_tls_start, &_tls_end, &_tls_index, &__xl_a, 0, 0} ;
#endif
