
#include <foobar/Common.hxx>
#include <foobar/Win32Cmdline.hxx>
#include <foobar/Win32Stream.hxx>
#include <foobar/StringTool.hxx>
#include <libpe.h>
#include <libhash/Fortuna.h>
#include "stbinfo.h"

extern "C" int LZSS_Compress(void* _in_b, int in_b_len, void* _out_b, int out_b_len);

void lzss_compress(std::vector<uint8_t>& buf)
{
	std::vector<uint8_t> outbuf(buf.size());
	int r = LZSS_Compress(&buf[0],buf.size(),&outbuf[0],outbuf.size());
	if ( r < 0 ) 
		throw std::runtime_error("core image is uncompressable");
	outbuf.resize(r);
	buf.swap(outbuf);
}

struct NEWDES
{
	static uint8_t Rotor(uint8_t a)
	{
		static uint8_t rotor[] =
		{
			32, 137, 239, 188, 102, 125, 221, 72, 212, 68, 81, 37, 86, 237, 147, 149,
			70, 229, 17, 124, 115, 207, 33, 20, 122, 143, 25, 215, 51, 183, 138, 142,
			146, 211, 110, 173,  1, 228, 189, 14, 103, 78, 162, 36, 253, 167, 116, 255,
			158, 45, 185, 50, 98, 168, 250, 235, 54, 141, 195, 247, 240, 63, 148,  2,
			224, 169, 214, 180, 62, 22, 117, 108, 19, 172, 161, 159, 160, 47, 43, 171,
			194, 175, 178, 56, 196, 112, 23, 220, 89, 21, 164, 130, 157,  8, 85, 251,
			216, 44, 94, 179, 226, 38, 90, 119, 40, 202, 34, 206, 35, 69, 231, 246,
			29, 109, 74, 71, 176,  6, 60, 145, 65, 13, 77, 151, 12, 127, 95, 199,
			57, 101,  5, 232, 150, 210, 129, 24, 181, 10, 121, 187, 48, 193, 139, 252,
			219, 64, 88, 233, 96, 128, 80, 53, 191, 144, 218, 11, 106, 132, 155, 104,
			91, 136, 31, 42, 243, 66, 126, 135, 30, 26, 87, 186, 182, 154, 242, 123,
			82, 166, 208, 39, 152, 190, 113, 205, 114, 105, 225, 84, 73, 163, 99, 111,
			204, 61, 200, 217, 170, 15, 198, 28, 192, 254, 134, 234, 222,  7, 236, 248,
			201, 41, 177, 156, 92, 131, 67, 249, 245, 184, 203,  9, 241,  0, 27, 46,
			133, 174, 75, 18, 93, 209, 100, 120, 76, 213, 16, 83,  4, 107, 140, 52,
			58, 55,  3, 244, 97, 197, 238, 227, 118, 49, 79, 230, 223, 165, 153, 59
		};
		return rotor[a];
	}

	struct Encipher
	{
		uint8_t unrav[68];

		Encipher(const uint8_t key[15])
		{
			int i, j;
			for (j = 0; j < 60;)
				for (i = 0; i < 15; ++i)
					unrav[j++] = key[i];
		}

		void Block8(uint8_t b[8])
		{
			const uint8_t* k = unrav;
			int count;

			for (count = 8; count--;)
			{
				b[4] = b[4] ^ Rotor(b[0] ^ * (k++));
				b[5] = b[5] ^ Rotor(b[1] ^ * (k++));
				b[6] = b[6] ^ Rotor(b[2] ^ * (k++));
				b[7] = b[7] ^ Rotor(b[3] ^ * (k++));

				b[1] = b[1] ^ Rotor(b[4] ^ * (k++));
				b[2] = b[2] ^ Rotor(b[4] ^ b[5]);
				b[3] = b[3] ^ Rotor(b[6] ^ * (k++));
				b[0] = b[0] ^ Rotor(b[7] ^ * (k++));
			}
			b[4] = b[4] ^ Rotor(b[0] ^ * (k++));
			b[5] = b[5] ^ Rotor(b[1] ^ * (k++));
			b[6] = b[6] ^ Rotor(b[2] ^ * (k++));
			b[7] = b[7] ^ Rotor(b[3] ^ * (k++));
		}
	};
};

template < class Cipher >
void cipher_cbci_8(uint8_t* data, size_t block_count, Cipher& cipher)
{
	uint8_t q[8] = {0};
	for (size_t i = 0; i < block_count; ++i)
	{
		int j;
		for (j = 0; j < 8; ++j)
			*((uint8_t*)data + 8 * i + j) ^= *(q + j);
		cipher.Block8(data + 8 * i);
		memcpy(q, (uint8_t*)data + 8 * i, 8);
	}
}

template < class Cipher >
void cipher_ecb_8(uint8_t* data, size_t block_count, Cipher& cipher)
{
	for (size_t i = 0; i < block_count; ++i)
	{
		cipher.Block8(data + 8 * i);
	}
}

uint32_t sq_rand(uint32_t* Sq)
{
	return *Sq = (((1664525L * (*Sq)) & (0x0ffffffff)) + 1013904223L) & (0x0ffffffff);
}

std::vector<uint8_t> copy_section(PE_SOURCE* pe, char* name)
{
	size_t no = Libpe_Find_Section_No(pe, name);
	if (no == PE_INVALID_NO)
		throw std::runtime_error(std::string("section not found: ") + Libpe_Last_Error_String());
	std::vector<uint8_t> ret(Libpe_Section_Raw_Size(pe, no));
	if (!Libpe_Copy_Offs(pe,Libpe_Section_Offs(pe, no), &ret[0], ret.size()))
		throw std::runtime_error(std::string("section not copyed: ") + Libpe_Last_Error_String());
		                         return ret;
	}

std::string opt_dflt(foobar::Cmdline cmdl, const char* opt, const char* dflt)
{
	if (cmdl->opt[opt].Specified())
		return cmdl->opt[opt].Str();
	else
		return std::string(dflt);
}

typedef std::unique_ptr<PE_SOURCE, PE_BOOL(*)(PE_SOURCE*)> PeSource;

int wmain(int argc, wchar_t** argv)
{
	try
	{
		foobar::Cmdline cmdl = foobar::Win32Cmdline::Parse("?|h,ldr:,in:,out:,noldr,nocor", argv, argc);
		int i, j;

		std::string ldr_name = opt_dflt(cmdl, "ldr", "loader.lso");
		PeSource ldr_pe(Libpe_Open_File_U(ldr_name.c_str()), Libpe_Close);
		if (!ldr_pe.get())
			throw std::runtime_error(Libpe_Last_Error_String());

		std::vector<uint8_t> ldr = copy_section(ldr_pe.get(), ".extra");
		while (ldr.size() % 8 != 0) ldr.push_back(0);

		if (auto fw = foobar::Win32Stream::Open(ldr_name + ".BIN", "w+"))
		{
			fw->Write(ldr);
			if (!fw->Good())
				throw std::runtime_error(fw->ErrorString());
		}

		if (!cmdl->opt["noldr"].Specified())
		{
			if (auto fw = foobar::Win32Stream::Open(ldr_name + ".BIN.c", "w+"))
			{
				if (!fw->Good())
					throw std::runtime_error(ldr_name + " : " +fw->ErrorString());
				fw->WriteLine("#include <stdint.h>");
				fw->WriteLine("uint8_t loader_BIN[] = {");
				i = 0;
				for (j = 0; j < ldr.size(); ++j, ++i)
				{
					if (i)
						fw->WriteText(", ");
					if (i && !(i % 9))
						fw->WriteLine();
					fw->WriteText(foobar::c_hex_byte(ldr[j], 'x').Cstr());
				}
				fw->WriteLine("};");
				fw->WriteLine("size_t loader_BIN_size = sizeof(loader_BIN);");
			}
		}

		if (!cmdl->opt["nocor"].Specified())
		{
			std::string corin_name = opt_dflt(cmdl, "in", "..\\.bin\\cor1rel.dll");
			std::string corout_name = opt_dflt(cmdl, "out", "molebox.BIN");

			PeSource cor_pe(Libpe_Open_File_U(corin_name.c_str()), Libpe_Close);
			if (!cor_pe.get())
				throw std::runtime_error(corin_name + " : " + Libpe_Last_Error_String());

			size_t text = Libpe_Find_Section_No(cor_pe.get(), ".text");
			if (text == PE_INVALID_NO)
				throw std::runtime_error(Libpe_Last_Error_String());
			uint64_t text_rva = Libpe_Section_RVA(cor_pe.get(), text);
			size_t text_size = Libpe_Section_Raw_Size(cor_pe.get(), text);
			size_t relo = Libpe_Find_Section_No(cor_pe.get(), ".reloc");
			if (relo == PE_INVALID_NO)
				throw std::runtime_error(Libpe_Last_Error_String());
			uint64_t relo_rva = Libpe_Section_RVA(cor_pe.get(), relo);
			size_t relo_size = Libpe_Section_Raw_Size(cor_pe.get(), relo);

			assert(text_rva >= relo_size + sizeof(CORE0_HEADER));

			CORE0_HEADER cor;
			memset(&cor, 0, sizeof(cor));
			std::vector<uint8_t> out(text_rva+text_size);
			cor.sign[0] = CORE0_SIGN_FIRST;
			cor.sign[1] = CORE0_SIGN_SECOND;
			cor.image_base  = Libpe_Get_Imagebase(cor_pe.get());
			cor.relocs_size = relo_size;
			cor.relocs = sizeof(cor);
			cor.imports_size = 0;
			cor.imports = 0;
			cor.build_number = 9999;

			uint64_t rva;
			if ((rva = Libpe_Find_Export_RVA(cor_pe.get(), "_SDK_Init@28")) == PE_INVALID_RVA)
				throw std::runtime_error("tehre is no enrty _SDK_Init@28");
			cor.sdkinit = rva;
			if ((rva = Libpe_Find_Export_RVA(cor_pe.get(), "_SDK_Inject@20")) == PE_INVALID_RVA)
				throw std::runtime_error("tehre is no enrty _SDK_Inject@20");
			cor.inject = rva;
			if ((rva = Libpe_Find_Export_RVA(cor_pe.get(), "_Inity@16")) == PE_INVALID_RVA)
				throw std::runtime_error("tehre is no enrty _Inity@16");
			cor.inity = rva;

			if (!Libpe_Copy_RVA(cor_pe.get(), relo_rva, &out[0] + sizeof(CORE0_HEADER), relo_size))
				throw std::runtime_error(Libpe_Last_Error_String());
			
			if (!Libpe_Copy_RVA(cor_pe.get(), text_rva, &out[0] + text_rva, text_size))
				throw std::runtime_error(Libpe_Last_Error_String());
			
			memcpy(&out[0],&cor,sizeof(cor));

			lzss_compress(out);

			uint8_t key[16];
			libhash::fortuna(key,16);
			NEWDES::Encipher cipher(key);
			uint8_t padding = out.size() % 8;
			if (!padding) padding = 8;
			std::fill_n(std::back_insert_iterator<std::vector<uint8_t>>(out), padding, padding);
			cipher_ecb_8(&out[0], out.size() / 8, cipher);

			if (*(uint32_t*)(&ldr[0x10]) != 0x4b495551 || *(uint32_t*)(&ldr[0x14]) != 0x31584f42)
				throw std::runtime_error("couldn't find 'QUIKBOX1' sign");

			*(uint32_t*)(&ldr[0x10]) = ldr.size();
			*(uint32_t*)(&ldr[0x14]) = out.size();
			memcpy(&ldr[0x18], key, 15);

			if (auto fw = foobar::Win32Stream::Open(corout_name, "w+"))
			{
				if (!fw->Good())
					throw std::runtime_error(corout_name + " : " + fw->ErrorString());
				fw->Write(ldr);
				fw->Write(out);
			}

			if (auto fw = foobar::Win32Stream::Open(corout_name + ".C", "w+"))
			{
				if (!fw->Good())
					throw std::runtime_error(corout_name + " : " + fw->ErrorString());
				fw->WriteLine("#include <stdint.h>");
				fw->WriteLine("uint8_t molebox_BIN[] = {");
				for (i = 0, j = 0; j < ldr.size(); ++j, ++i)
				{
					if (i)
						fw->WriteText(", ");
					if (i && !(i % 8))
						fw->WriteLine();
					fw->WriteText(foobar::c_hex_byte(ldr[j], 'x').Cstr());
				}
				for (j = 0; j < out.size(); ++j, ++i)
				{
					if (i)
						fw->WriteText(", ");
					if (i && !(i % 8))
						fw->WriteLine();
					fw->WriteText(foobar::c_hex_byte(out[j], 'x').Cstr());
				}
				fw->WriteLine("};");
				fw->WriteLine("size_t molebox_BIN_size = sizeof(molebox_BIN);");
			}
		}
		return 0;
	}
	catch (std::exception& e)
	{
		fputws(foobar::wcsarg_t(e.what()).Cstr(),stderr);
		return -1;
	}
}

