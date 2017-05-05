
#include <stdint.h>
#include "nt.h"
#include "stbinfo.h"
#include "fwinstall.h"

typedef long long INT64T;
#include "xojuman.h"

__attribute__((section(".text$code")))
void memcopy(uint8_t* to, uint8_t* from, int count)
{
	while (count--)
		*to++ = *from++;
}

__attribute__((section(".text$code")))
void* S_Decrypt(char* b, uint8_t* S)
{
	char* p = b;
	uint8_t Q = *S++;

	do
	{
		*p = (*S++)^Q;
		Q = *p;
		Q = (Q << 4) | (Q >> 4);
	}
	while (*p++);

	return b;
}

__attribute__((section(".text$code")))
void* Query_Core_Key(void* b)
{
	int i ;
	char* key = _Self_Key();
	for (i = 0; i < 15; ++i)((char*)b)[i] = key[i];
	return b;
}

__attribute__((section(".text$code")))
uint8_t Xor_Byte_W(uint8_t* o, uint8_t q, uint8_t w)
{
	*o = *o ^ q;
	return w;
}

__attribute__((section(".text$code")))
void Do_Cipher_CBCO(uint8_t* key_unrav, void* data, uint32_t count)
{
	uint8_t q[8] = {0};
	int i, j;
	for (i = 0; i < count; ++i)
	{
		uint8_t w[8];
		memcpy(w, (uint8_t*)data + 8 * i, 8);
		Do_Cipher(key_unrav, (uint8_t*)data + 8 * i, 1);
		for (j = 0; j < 8; ++j)
			q[j] = Xor_Byte_W(((uint8_t*)data + 8 * i + j), q[j], w[j]);
		//*((uint8_t*)data+BLOCK_BYTES*i+4) ^= q[4]; q[4] = w[4];
		//*((uint8_t*)data+BLOCK_BYTES*i+1) ^= q[1]; q[1] = w[1];
		//*((uint8_t*)data+BLOCK_BYTES*i+5) ^= q[5]; q[5] = w[5];
		//*((uint8_t*)data+BLOCK_BYTES*i+0) ^= q[0]; q[0] = w[0];
		//*((uint8_t*)data+BLOCK_BYTES*i+6) ^= q[6]; q[6] = w[6];
		//*((uint8_t*)data+BLOCK_BYTES*i+2) ^= q[2]; q[2] = w[2];
		//*((uint8_t*)data+BLOCK_BYTES*i+7) ^= q[7]; q[7] = w[7];
		//*((uint8_t*)data+BLOCK_BYTES*i+3) ^= q[3]; q[3] = w[3];
	}
}

__attribute__((section(".text$code")))
void* Unpack_Core(void* self, FUNCS* f, int* crc, void** cor_handle)
{
	int i, rsz;
	uint8_t K[16];
	uint8_t unrav[68];
	void* mmap = f->Create_File_Mapping((void*) - 1, 0, PAGE_READWRITE, 0, _Core_Size(), 0);
	void* q = f->Map_View_Of_File(mmap, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	char* r = 0; //, *q = VA(0,_Core_Size(),MEM_COMMIT,PAGE_READWRITE);
	uint8_t* DT = _Query_DT64();
	int qsize = _Core_Size();
	uint8_t padding = 0;
	memcopy(q, (char*)self + _Core_Offset(), _Core_Size());
	Setup_Decipher(unrav, Query_Core_Key(K));
	Do_Cipher(unrav, q, qsize / 8);
	rsz = *(int*)q;
	r = f->Virtual_Alloc(0, rsz, MEM_COMMIT, PAGE_EXECUTE_READWRITE);
	if (!r) r = f->Virtual_Alloc(0, rsz, MEM_COMMIT, PAGE_READWRITE);
	padding = ((uint8_t*)q)[qsize - 1];
	qsize -= padding;
	rsz = LZSS_Decompress(q + 4, qsize - 4, r, rsz);
	if (rsz <= 0) goto Err;
	*crc = adler32(0, r + 0x2000, rsz - 0x2000);
	*cor_handle = mmap;
	f->Unmap_View_Of_File(q);
	return r;

Err:
	if (mmap) f->Close_Handle(mmap);
	if (q) f->Unmap_View_Of_File(q);
	if (r) f->Virtual_Free(r, 0, MEM_RELEASE);
	return 0;
}

__attribute__((section(".text$code")))
void Relocate_Core(CORE0_HEADER* core)
{
	uint8_t* base = (void*)core;
	unsigned offs = (longptr_t)core - core->image_base;
	uint8_t* reloc  = (uint8_t*)core + core->relocs;
	uint8_t* relocE = reloc + core->relocs_size;

	while (reloc < relocE)
	{
		int rva = *(int*)reloc; reloc += 4;
		unsigned bsize = *(unsigned*)reloc - 8; reloc += 4;
		uint8_t const* e = reloc + bsize;
		while (reloc < e)
		{
			switch (*(unsigned short*)reloc >> 12)
			{
				default:
					__asm__(("int3"));
				case 3:
				{
					int fixup = *(unsigned short*)reloc & 0x0fff;
					*(unsigned*)(base + rva + fixup) += offs;
				}
				case 0:
					;
			}
			reloc += 2;
		}
	}
}

#define _BASE() ( ((void**)_PEB())[2] )
#define _PROCENV() ( ((void***)_PEB())[4][18] )
#define PEB_LDR(x) ( *(int*)((char*)x + 0xc) )
#define FLINK(x) ( ((int*)x)[0] )
#define BLINK(x) ( ((int*)x)[1] )
#define DLO_BASE(x) ( ((int*)x)[24/4] )
#define DLO_NAME(x) ( ((unsigned int*)x)[40/4] )
#define DLO_NLEN(x) ( ((unsigned int*)x)[36/4] & 0x0ff )

__attribute__((section(".text$code")))
void* Find_Dll(int T)
{
	void* handle = 0;
	int l = _DLO();
	int S = FLINK(l);

	while (S != l)
	{
		if (T == 0 && ISNTDLL(S))
		{
			handle = (void*)DLO_BASE(S);
			break;
		}
		else if (T == 1 && ISK32DLL(S))
		{
			handle = (void*)DLO_BASE(S);
			break;
		}
		S = FLINK(S);
	}

	return handle;
}

__attribute__((section(".text$code")))
int seql(char* a, char* b)
{
	while (*a && *b && *a == *b) ++a, ++b;
	return *a == *b;
}

__attribute__((section(".text$code")))
void* Find_Func(void* handle, char* name)
{
	int i = 0, fndx;
	IMAGE_NT_HEADERS* nth = NT_HEADERS(handle);
	IMAGE_DATA_DIRECTORY expo = nth->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
	IMAGE_EXPORT_DIRECTORY* edir = (IMAGE_EXPORT_DIRECTORY*)((char*)handle + expo.VirtualAddress);
	for (i = 0; i < edir->NumberOfNames; ++i)
	{
		if (!seql(name, (char*)handle + ((unsigned int*)((char*)handle + edir->AddressOfNames))[i]))
			continue;
	hinted:
		fndx = ((unsigned short*)((char*)handle + edir->AddressOfNameOrdinals))[i];
		return
		    ((char*)handle +
		     ((unsigned int*)((char*)handle + edir->AddressOfFunctions))[fndx]);
	}
	return 0;
}

__attribute__((section(".text$code")))
void* Find_GPA(void* ntdll, void* kernl)
{
	char b[64];
	ANSI_STRING S = { 14, 15, b };
	void *(*LGPA)() = Find_Func(ntdll, S_Decrypt(b, _S(1399548921, "LdrGetProcedureAddress")));
	void* GPA = 0;
	S_Decrypt(b, _S(1400269796, "GetProcAddress"));
	LGPA(kernl, &S, 0, &GPA);
	return GPA;
}

__attribute__((section(".text$code")))
wchar_t *Find_Module_Name(FUNCS *f)
{
	int l = _DLO();
	int S = l;//FLINK(l);
	wchar_t *pS = (wchar_t*)_PROCENV();
	wchar_t *p  = pS;
	wchar_t b[10];

	S_Decrypt((char*)b, _S(1398434760, "X\0O\0J\0U\0M\0A\0N\0=\0"));
	
	while (*p)
	{
		wchar_t *pS = p;
		while (*p++);
		if ( !memcmp(pS,b,8*2) )
		{
			f->injected = 1;
			return f->modname = pS+8;
		}
	}

	do
	{
		if ( DLO_BASE(S) == (unsigned)_BASE() )
			return f->modname = (wchar_t*)DLO_NAME(S);
		S = FLINK(S);
	}
	while ( S != l );

	return 0;
}

__attribute__((section(".text$code")))
void Get_Funcs(FUNCS* f, void* kernl, GPA_t GPA)
{
	char bf[64];
	memset(f,0,sizeof(*f));

	f->Get_Proc_Address     = GPA;
	f->Load_Library         = GPA(kernl, S_Decrypt(bf, _S(1398434760, "LoadLibraryA")));
	f->Get_Module_Handle    = GPA(kernl, S_Decrypt(bf, _S(1395289016, "GetModuleHandleA")));
	f->Virtual_Alloc        = GPA(kernl, S_Decrypt(bf, _S(1396599724, "VirtualAlloc")));
	f->Virtual_Free         = GPA(kernl, S_Decrypt(bf, _S(1393126299, "VirtualFree")));
	f->Get_Env_Value        = GPA(kernl, S_Decrypt(bf, _S(1394109322, "GetEnvironmentVariableA")));
	f->Create_File_Mapping  = GPA(kernl, S_Decrypt(bf, _S(1407478646, "CreateFileMappingA")));
	f->Map_View_Of_File     = GPA(kernl, S_Decrypt(bf, _S(1405709147, "MapViewOfFile")));
	f->Unmap_View_Of_File   = GPA(kernl, S_Decrypt(bf, _S(1407019855, "UnmapViewOfFile")));
	f->Close_Handle         = GPA(kernl, S_Decrypt(bf, _S(1403480885, "CloseHandle")));
	f->Map_View_Of_File_Ex  = GPA(kernl, S_Decrypt(bf, _S(1404398375, "MapViewOfFileEx")));
	f->Create_File          = GPA(kernl, S_Decrypt(bf, _S(1404398375, "CreateFileW")));
	f->Set_File_Pointer     = GPA(kernl, S_Decrypt(bf, _S(1404398375, "SetFilePointer")));
	f->Read_File            = GPA(kernl, S_Decrypt(bf, _S(1404398375, "ReadFile")));

	if ( Find_Module_Name(f) )
		f->eh = f->Create_File(f->modname,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
}

__attribute__((section(".text$code")))
void* Molebox_Install(void* self, unsigned flags, void* key, void* offs)
{
	char bf[64];
	int load_external = 0;
	void* cor_mapping = 0;
	int   cor_size = 0;
	void* core = 0;
	void *(__stdcall * Core_Entry)(int, void*, void*, void*, void*, void*, int) = 0;

	void* ntdll     = Find_Dll(0);
	void* kernl     = Find_Dll(1);
	GPA_t GPA       = Find_GPA(ntdll, kernl);

	FUNCS funcs;
	Get_Funcs(&funcs, kernl, GPA);

	load_external = funcs.Get_Env_Value(S_Decrypt(bf, _S(1402956557, "MOLEBOX_EXTERNAL_CORE_1376")), 0, 0);

	if (load_external)
	{
		core = funcs.Load_Library(S_Decrypt(bf, _S(1384016620, "cor1dbg.dll")));
		Core_Entry = GPA(core, S_Decrypt(bf, _S(1380739806, "_SDK_Init@28")));
	}
	else
	{
		int crc;
		core = Unpack_Core(self, &funcs, &crc, &cor_mapping);
		cor_size = _Core_Size();
		Core_Entry = (void*)((uint8_t*)core + ((CORE0_HEADER*)core)->sdkinit);
		Relocate_Core(core);
	}

	return Core_Entry(flags, ntdll, kernl, core, key, cor_mapping, cor_size);
}

__attribute__((section(".text$code")))
int Seek_File(FUNCS* f, unsigned offs)
{
	unsigned foo = 0;
	return f->Set_File_Pointer(f->eh, offs, &foo, 0);
}

__attribute__((section(".text$code")))
int Read_File(FUNCS *f, void *ptr, unsigned len)
{
	unsigned foo = 0;
	do
	{
		if ( f->Read_File(f->eh,ptr,len,&foo,0) && foo > 0 )
			*(char**)&ptr += foo, len-=foo;
		else
			return 0;
	}
	while(len);
	return 1;
}

__attribute__((section(".text$code")))
void* Molebox_Init(void* _always_zero_)
{
	STB_INFO2* nfo = 0;
	char bf[64];
	int load_external = 0;
	void* cor_mapping = 0;
	int   cor_size = 0;
	void* core = 0;
	void *(__stdcall * Core_Entry)(void*, void*, STB_INFO*, void*) = 0;

	void* ntdll     = Find_Dll(0);
	void* kernl     = Find_Dll(1);
	GPA_t GPA       = Find_GPA(ntdll, kernl);

	uint8_t K[16];
	uint8_t unrav[68];
	FUNCS funcs;
	Get_Funcs(&funcs, kernl, GPA);

	load_external = funcs.Get_Env_Value(S_Decrypt(bf, _S(1381984973, "MOLEBOX_EXTERNAL_CORE_1376")), 0, 0);

	if (load_external)
	{
		core = funcs.Load_Library(S_Decrypt(bf, _S(1379691182, "cor1dbg.dll")));
		Core_Entry = GPA(core, S_Decrypt(bf, _S(1375890064, "_Inity@16")));
	}
	else
	{
		int crc;
		void* self = _Self_Ptr();
		core = Unpack_Core(self, &funcs, &crc, &cor_mapping);
		cor_size = _Core_Size();
		Core_Entry = (void*)((uint8_t*)core + ((CORE0_HEADER*)core)->inity);
		Relocate_Core(core);
		funcs.Close_Handle(cor_mapping); /* for now */
	}

	nfo = funcs.Virtual_Alloc(0, sizeof(STB_INFO2), MEM_COMMIT, PAGE_READWRITE);

	if (1)
	{
		uint32_t sign[2];
		unsigned mask, i, fa;
		IMAGE_NT_HEADERS const* nth = NT_HEADERS(_BASE());
		IMAGE_SECTION_HEADER const* S = (IMAGE_SECTION_HEADER*)((char*)nth + (24 + nth->FileHeader.SizeOfOptionalHeader));

		fa = (nth->OptionalHeader.FileAlignment - 1);
		S += nth->FileHeader.NumberOfSections - 1;
		while (!S->PointerToRawData) --S;
		i = (S->PointerToRawData + S->SizeOfRawData + fa)&~fa;

		if (!Seek_File(&funcs, i) || !Read_File(&funcs, sign, 8))
			return 0;
		if (((sign[1] ^ STB_SIGN_SECOND) ^ sign[0]) != STB_SIGN_FIRST)
			return 0;
	}

	Setup_Decipher(unrav, nfo->i.xcrypt_key);
	Do_Cipher_CBCO(unrav, &nfo->i, (sizeof(nfo->i) - 16) / 8);
	Do_Cipher_CBCO(unrav, &nfo->i + 1, (sizeof(nfo) - sizeof(nfo->i)) / 8);

	return Core_Entry(ntdll, kernl, &nfo->i, core);
}

enum
{
    LZSS_MIN_LEN = 2,
    LZSS_MAX_LEN = 15,
    LZSS_TABLE_MOD = 4095, /*4095, */
    LZSS_TABLE_LEN = LZSS_TABLE_MOD + 1,
};

__attribute__((section(".text$code")))
int LZSS_Decompress(uint8_t* in_b, int in_b_len, char* out_b, int out_b_len)
{
	int in_i = 0;
	int out_i = 0;
	while (in_i < in_b_len && out_i < out_b_len)
	{
		if (in_b[in_i] == 0x80)
		{
			// one char
			out_b[out_i++] = in_b[++in_i];
			++in_i;
		}
		else if (!in_b[in_i])
		{
			// several chars
			int l = (int)in_b[++in_i] + 1;
			++in_i;
			while (l--)
			{
				out_b[out_i++] = in_b[in_i++];
			}
		}
		else
		{
			// code
			unsigned short code = (short)in_b[in_i] | ((short)in_b[in_i + 1] << 8);
			int l = code & 0x0f;
			int off = code >> 4;
			memcopy(out_b + out_i, out_b + out_i - off - LZSS_MAX_LEN, l);
			out_i += l;
			in_i += 2;
		}
	}

	return out_i;
}

enum { BLOCK_BYTES = 8 };
enum { KEY_BYTES   = 15 };
enum { UNRAV_BYTES = 60 };
enum { UNRAV_BYTES_1 = 68 };

__attribute__((section(".text$code")))
void _Normalize_(uint8_t* key_unrav, uint8_t const* kunrav)
{
	int i, j;
	memset(key_unrav, 0, 68);
	for (i = 0; i < 9; ++i)
	{
		for (j = 0; j < 4; ++j)
			key_unrav[i * 8 + j] = kunrav[i * 7 + j];
		if (i < 8)
		{
			key_unrav[i * 8 + j]   = kunrav[i * 7 + j]; ++j;
			key_unrav[i * 8 + j + 1] = kunrav[i * 7 + j]; ++j;
			key_unrav[i * 8 + j + 1] = kunrav[i * 7 + j]; ++j;
		}
	}
}

__attribute__((section(".text$code")))
void Setup_Decipher(uint8_t* key_unrav, void* _key)
{
	uint8_t kunrav[UNRAV_BYTES];
	uint8_t* k = kunrav;
	uint8_t* key = _key;
	int i = 11, j = 0;
	while (1)
	{
		*(k++) = key[i];
		i = ((i + 1) % KEY_BYTES);
		*(k++) = key[i];
		i = ((i + 1) % KEY_BYTES);
		*(k++) = key[i];
		i = ((i + 1) % KEY_BYTES);

		*(k++) = key[i];
		i = (i + 9) % 15;
		if (i == 12) break;

		*(k++) = key[i++];
		*(k++) = key[i++];
		*(k++) = key[i];
		i = (i + 9) % 15;
	}
	_Normalize_(key_unrav, kunrav);
}

#define _ADLER32_DO1(buf,i)  {s1 += buf[i]; s2 += s1;}
#define _ADLER32_DO2(buf,i)  _ADLER32_DO1(buf,i); _ADLER32_DO1(buf,i+1);
#define _ADLER32_DO4(buf,i)  _ADLER32_DO2(buf,i); _ADLER32_DO2(buf,i+2);
#define _ADLER32_DO8(buf,i)  _ADLER32_DO4(buf,i); _ADLER32_DO4(buf,i+4);
#define _ADLER32_DO16(buf)   _ADLER32_DO8(buf,0); _ADLER32_DO8(buf,8);

__attribute__((section(".text$code")))
unsigned adler32(unsigned adler, uint8_t* buf, unsigned len)
{
	enum
	{
	    BASE = 65521L, /* largest prime smaller than 65536 */
	    /* NMAX is the largest n such that 255n(n+1)/2 + (n+1)(BASE-1) <= 2^32-1 */
	    NMAX = 5552
	};

	unsigned s1 = adler & 0xffff;
	unsigned s2 = (adler >> 16) & 0xffff;
	int k;

	if (buf == 0) return 1L;

	while (len > 0)
	{
		k = len < NMAX ? len : NMAX;
		len -= k;
		while (k >= 16)
		{
			_ADLER32_DO16(buf);
			buf += 16;
			k -= 16;
		}
		if (k != 0)
			do
			{
				s1 += *buf++;
				s2 += s1;
			}
			while (--k);
		s1 %= BASE;
		s2 %= BASE;
	}
	return (s2 << 16) | s1;
}

__attribute__((section(".text$code")))
void* Unpack_Core_2(XOJUMAN_STARTUP_INFO* sti, FUNCS* f, int* crc)
{
	int i, rsz;
	unsigned cor_addr = 0x6F880000;
	void* q = f->Map_View_Of_File(sti->core_handle, FILE_MAP_WRITE | FILE_MAP_READ, 0, 0, 0);
	char* r = 0; //, *q = VA(0,_Core_Size(),MEM_COMMIT,PAGE_READWRITE);

	if (rsz = *(int*)q)
	{
		void* mmap;
		mmap = f->Create_File_Mapping((void*) - 1, 0, PAGE_EXECUTE_READWRITE, 0, rsz, 0);
		if (!mmap) mmap = f->Create_File_Mapping((void*) - 1, 0, PAGE_READWRITE, 0, rsz, 0);
		while (!r)
		{
			r = f->Map_View_Of_File_Ex(mmap, FILE_MAP_WRITE | FILE_MAP_READ | 0x20, 0, 0, 0, (void*)cor_addr);
			cor_addr += 0x10000;
		}
	}

	rsz = LZSS_Decompress(q + 4, sti->core_size - 4, r, rsz);
	if (rsz <= 0) goto Err;
	*crc = adler32(0, r + 0x2000, rsz - 0x2000);
	f->Unmap_View_Of_File(q);
	return r;

Err:
	if (q) f->Unmap_View_Of_File(q);
	if (r) f->Unmap_View_Of_File(r);
	return 0;
}

__attribute__((section(".text$code")))
void Molebox_Inject(XOJUMAN_STARTUP_INFO* sti)
{
	//__asm__(("int3"));

	char bf[64];
	void* ntdll = Find_Dll(0);
	void* kernl = Find_Dll(1);
	GPA_t GPA   = Find_GPA(ntdll, kernl);
	FUNCS funcs = {0};
	void* core  = 0;
	void *(__stdcall * Core_Entry)(int, void*, void*, void*, void*) = 0;

	Get_Funcs(&funcs, kernl, GPA);

	if (!sti->core_handle)
	{
		core = funcs.Load_Library(S_Decrypt(bf, _S(1377790605, "cor1dbg.dll")));
		Core_Entry = GPA(core, S_Decrypt(bf, _S(1391291007, "_SDK_Inject@20")));
	}
	else
	{
		int crc;
		core = Unpack_Core_2(sti, &funcs, &crc);
		Core_Entry = (void*)((uint8_t*)core + ((CORE0_HEADER*)core)->inject);
		Relocate_Core(core);
	}

	((void**)&sti)[-1] = Core_Entry(sti->flags, ntdll, kernl, core, sti);
}

