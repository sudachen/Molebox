
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"
#include <time.h>
#include "stbinfo.h"
#include "teggovbox.h"
#include "peover.h"
#include <rsa.h>

extern "C" unsigned char core0S[];
extern "C" unsigned core0S_size;

bool AfxBundleDLL(StringParam dllname, StringA& module, StringParam target, BUNDLED_IMPORT& bi, byte_t* xcrypt_key,
                  EhFilter ehf)
{

    unsigned rel_length = LengthOfFile(dllname);
    unsigned crc = 0;
    unsigned offset = 0;
    int pak_length;
    BufferT<byte_t> data(rel_length);
    BufferT<byte_t> cdata(rel_length+4);

    DataStreamPtr ds = OpenHandleStreamRtry(dllname,STREAM_READ|STREAM_NONINHERITABLE);
    if ( !ds )
    { ehf(_S*"failed to open module '%s'"%dllname); return false; }

    ds->Read(+data,data.Count());
    Unrefe(ds);

    if ( (pak_length = zlib_compress(+data,rel_length,+cdata+4,rel_length) ) > 0 )
    {
        cdata.Swap(data);
        bi.zipped = 1;
        pak_length += 4;
        *(unsigned*)+data = rel_length;
    }
    else
        pak_length = rel_length;

    bi.xcrypt = 1;
    NEWDES_Cipher cipher;
    cipher.SetupEncipher(xcrypt_key);
    cipher.DoCipher(+data,pak_length/8);
    bi.crc = Crc32(0,+data,pak_length);
    ds = OpenStreamRtry(target,STREAM_WRITE|STREAM_NONINHERITABLE);
    ds->Seek(0,SEEK_END);
    bi.offset = ds->Tell();
    ds->Write(+data,pak_length);
    Unrefe(ds);

    {
        u32_t r = GetMillis();
        for ( int i = 0; i < sizeof(bi.name)/4; ++i )
            *(unsigned*)(bi.name+i*4) = teggo::unsigned_random(&r);
        bi.name[sizeof(bi.name)-1] = 0;
        strncpy(bi.name,+module,sizeof(bi.name)-1);
    }

    bi.count = pak_length;
    bi.size  = rel_length;
    return true;
}

bool AfxAttachBundles(StringParam target, BufferT<BUNDLED_IMPORT>& bundles, byte_t* xcrypt_key, OCC& info_bundle, EhFilter ehf)
{
    if ( bundles.Count() )
    {
        BUNDLE_HEADER hdr = {0};
        hdr.number = bundles.Count();
        DataStreamPtr ds = OpenHandleStreamRtry(target,STREAM_WRITE|STREAM_NONINHERITABLE);
        ds->Seek(0,SEEK_END);
        unsigned offset = ds->Tell();
        ds->Write(&hdr,sizeof(hdr));
        Qsort(bundles.Begin(),bundles.End(),LessBundle());
        BufferT<byte_t> edata(bundles.Count()*sizeof(bundles[0]));
        memcpy(+edata,+bundles,edata.Count());
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(xcrypt_key);
        cipher.DoCipherCBCI(+edata,edata.Count()/8);
        ds->Write(+edata,edata.Count());
        Unrefe(ds);

        info_bundle.offset = offset;
        info_bundle.count  = bundles.Count()*sizeof(bundles[0])+sizeof(hdr);
        info_bundle.crc    = Crc32(0,(byte_t*)&hdr,sizeof(hdr));
        info_bundle.crc    = Crc32(info_bundle.crc,(byte_t*)+bundles,bundles.Count()*sizeof(bundles[0]));
    }
    return true;
}

bool AfxAttachSquoted(StringParam target, BufferT<SQUOTED_IMPORT>& squoted, byte_t* xcrypt_key, OCC& info_squoted, EhFilter ehf)
{
    SQUOTED_HEADER hdr = {0};
    unsigned offset = 0;

    if ( squoted.Count() )
    {
        hdr.number = squoted.Count();
        DataStreamPtr ds = OpenHandleStreamRtry(target,STREAM_WRITE|STREAM_NONINHERITABLE);
        ds->Seek(0,SEEK_END);
        offset = ds->Tell();
        ds->Write(&hdr,sizeof(hdr));
        BufferT<byte_t> edata(squoted.Count()*sizeof(squoted[0]));
        memcpy(+edata,+squoted,edata.Count());
        NEWDES_Cipher cipher;
        cipher.SetupEncipher(xcrypt_key);
        cipher.DoCipherCBCI(+edata,edata.Count()/8);
        ds->Write(+edata,edata.Count());
        Unrefe(ds);

        info_squoted.offset = offset;
        info_squoted.count  = squoted.Count()*sizeof(squoted[0])+sizeof(hdr);
        info_squoted.crc    = Crc32(0,(byte_t*)&hdr,sizeof(hdr));
        info_squoted.crc    = Crc32(info_squoted.crc,(byte_t*)+squoted,squoted.Count()*sizeof(squoted[0]));
    }
    return true;
}

bool AfxIsExecutable(StringParam name)
{
    if ( DataStreamPtr ds = OpenStreamRtry(name,STREAM_READ) )
    {
        IMAGE_DOS_HEADER dos = {0};
        IMAGE_NT_HEADERS nth = {0};

        ds->Seek(0);
        if ( sizeof(dos) == ds->Read(&dos,sizeof(dos)) )
            if ( dos.e_magic == IMAGE_DOS_SIGNATURE )
                if ( TEGGO_OK == ds->Seek(dos.e_lfanew) )
                    if ( sizeof(nth) == ds->Read(&nth,sizeof(nth)) )
                        if ( nth.Signature == IMAGE_NT_SIGNATURE )
                            if ((nth.FileHeader.Characteristics&(IMAGE_FILE_DLL|IMAGE_FILE_EXECUTABLE_IMAGE)) == IMAGE_FILE_EXECUTABLE_IMAGE)
                                return true;
    }
    return false;
}

StringW FindStub(StringParam s)
{
    if ( !s || !s.Length() ) return "";
    StringW p;

    p = AtCurrentDir(s);
    if ( DataSource->Exists(p) )
        return p;
    p = AtAppFolder(s);
    if ( DataSource->Exists(p) )
        return p;
    p = AtLocalUserAppFolder(s);
    if ( DataSource->Exists(p) )
        return p;
    p = AtUserAppFolder(s);
    if ( DataSource->Exists(p) )
        return p;
    if ( DataSource->Exists(s) )
        return +s;

    return "";
}


bool CopyStream(DataStream* from, DataStream* to, EhFilter ehf=EMPTYEF)
{
    BufferT<byte_t> b(64*1024);
    for (; from->Available();)
    {
        int q = from->Read(+b,b.Count());
        if ( q > 0 )
            to->Write(+b,q);
        else if ( q < 0 )
            return (ehf("failed to copy stream",false));
        //else
        //  break;
    }
    return true;
}

#include "../qNN.inl"

bool UnpackEmbeddedStub(BufferT<byte_t>& b,EhFilter ehf)
{
    unsigned size;
    byte_t* p = 0;

    int n = 0;
    size = stubS[n].size;
    p = stubS[n].data;

    unsigned sz = *(unsigned*)p;
    p += 4;
    unsigned crc = *(unsigned*)p;
    p += 4;
    size -= 8;
    b.Resize(sz);
    zlib_decompress(p,size,+b,sz);
    if ( Crc32(0,+b,sz) != crc )
    { ehf("embedded exe-template is corrupted"); return false; }
    return true;
}

DataStreamPtr CopyStub(StringParam from, StringParam to, void* _, EhFilter ehf)
{
    BufferT<byte_t> b;
    if ( !from.Length() )
    {
        UnpackEmbeddedStub(b,ehf);
    }
    else
    {
        if ( DataStreamPtr ds = OpenHandleStreamRtry(from,STREAM_READ|STREAM_NONINHERITABLE) )
            ds->Read(b,ds->Available());
        else
            return (ehf("failed to open exe-template"),DataStreamPtr(0));
    }

    if ( b.Count() )
    {
        if ( DataStreamPtr dd = OpenHandleStreamRtry(to,STREAM_WRITE|STREAM_CREATE|STREAM_NONINHERITABLE) )
        {
            dd->Write(+b,b.Count());
            return dd;
        }
        else
            return (ehf("failed to create target executable"),DataStreamPtr(0));
    }
    else
        ehf("failed to write target executable, no data");

    return DataStreamPtr(0);
}
