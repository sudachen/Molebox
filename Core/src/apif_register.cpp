
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"
#include "apif.h"
#include "import.h"
#include "splicer.h"
#include "logger.h"
#include "hoarray.h"
#include "vfs.h"
#include "XorS.h"
#include "xnt.h"
#include "teggovbox.h"

#include <sddl.h>

struct NT_DUPLICATE_ARGS
{
    HANDLE                SourceProcessHandle;
    HANDLE                SourceHandle;
    HANDLE                TargetProcessHandle;
    HANDLE*                TargetHandle;
    ACCESS_MASK           DesiredAccess;
    BOOLEAN               InheritHandle;
    ULONG                 Options;
};

struct NT_NOTYF_MULTKEY
{
    HANDLE                KeyHandle;
    ULONG                 Flags;
    OBJECT_ATTRIBUTES*     KeyObjectAttributes;
    HANDLE                EventHandle;
    /*PIO_APC_ROUTINE */ void* ApcRoutine;
    VOID*                  ApcContext;
    IO_STATUS_BLOCK*       IoStatusBlock;
    ULONG                 NotifyFilter;
    BOOLEAN               WatchSubtree;
    VOID*                  Buffer;
    ULONG                 BufferLength;
    BOOLEAN               Asynchronous;
};

struct NT_NOTYF_KEY
{
    HANDLE                KeyHandle;
    HANDLE                EventHandle;
    /*PIO_APC_ROUTINE */ void* ApcRoutine;
    VOID*                  ApcContext;
    IO_STATUS_BLOCK*       IoStatusBlock;
    ULONG                 NotifyFilter;
    BOOLEAN               WatchSubtree;
    VOID*                  Buffer;
    ULONG                 BufferLength;
    BOOLEAN               Asynchronous;
};

struct NT_CREATE_HKEY_ARGS
{
    HANDLE*                KeyHandle;
    ACCESS_MASK           DesiredAccess;
    OBJECT_ATTRIBUTES*     ObjectAttributes;
    ULONG                 TitleIndex;
    UNICODE_STRING*        Class;
    ULONG                 CreateOptions;
    ULONG*                 Disposition;
};

struct NT_OPEN_HKEY_ARGS
{
    HANDLE*                KeyHandle;
    ACCESS_MASK           DesiredAccess;
    OBJECT_ATTRIBUTES*     ObjectAttributes;
};

struct NT_SETVAL_HKEY_ARGS
{
    HANDLE                KeyHandle;
    UNICODE_STRING*        ValueName;
    ULONG                 TitleIndex;
    ULONG                 Type;
    VOID*                  Data;
    ULONG                 DataSize;
};

struct NT_ENUMKEY_HKEY_ARGS
{
    HANDLE                KeyHandle;
    ULONG                 Index;
    KEY_INFORMATION_CLASS KeyInformationClass;
    VOID*                  KeyInformation;
    ULONG                 Length;
    ULONG*                 ResultLength;
};

struct NT_ENUMVAL_HKEY_ARGS
{
    HANDLE                KeyHandle;
    ULONG                 Index;
    KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass;
    VOID*                  KeyValueInformation;
    ULONG                 Length;
    ULONG*                 ResultLength;
};

struct NT_QUERYKEY_HKEY_ARGS
{
    HANDLE                KeyHandle;
    KEY_INFORMATION_CLASS KeyInformationClass;
    VOID*                  KeyInformation;
    ULONG                 Length;
    ULONG*                 ResultLength;
};

struct NT_QUERYVAL_HKEY_ARGS
{
    HANDLE                KeyHandle;
    UNICODE_STRING*        ValueName;
    KEY_VALUE_INFORMATION_CLASS  KeyValueInformationClass;
    VOID*                  KeyValueInformation;
    ULONG                 Length;
    ULONG*                 ResultLength;
};

struct NT_CLOSE_ARGS
{
    HANDLE               FileHandle;
};

struct NT_DELKEY_ARGS
{
    HANDLE               KeyHandle;
};

struct NT_DELKEYVAL_ARGS
{
    HANDLE               KeyHandle;
    UNICODE_STRING*       ValueName;
};

struct NT_QUESEQ_ARGS
{
    HANDLE               ObjectHandle;
    SECURITY_INFORMATION SecurityInformationClass;
    SECURITY_DESCRIPTOR*  DescriptorBuffer;
    ULONG                DescriptorBufferLength;
    ULONG*                RequiredLength;
};

struct RegValue : Refcounted
{
    struct BINARYDATA { u32_t Length; byte_t* Buffer; };

    u32_t type_;
    StringW name_;

    union
    {
        u32_t DW;
        UNICODE_STRING S;
        BINARYDATA B;
    } value_;

    StringW const& Name() { return name_; }

    void _Clear()
    {
        switch (type_)
        {
            case REG_SZ:
            case REG_EXPAND_SZ:
            case REG_MULTI_SZ:
                _FREE(value_.S.Buffer);
                break;
            case REG_BINARY:
                _FREE(value_.B.Buffer);
                break;
        }
        type_ = REG_NONE;
    }

    byte_t* Data()
    {
        if ( type_ == REG_NONE )
            return 0;
        if ( type_ == REG_BINARY )
            return (byte_t*)value_.B.Buffer;
        if ( type_ == REG_DWORD )
            return (byte_t*)&value_.DW;
        if ( !value_.S.Buffer )
            return (byte_t*)L"";
        return (byte_t*)value_.S.Buffer;
    }

    unsigned DataLength()
    {
        if ( type_ == REG_NONE ) return 0;
        if ( type_ == REG_BINARY ) return value_.B.Length;
        if ( type_ == REG_DWORD ) return 4;
        return value_.S.Length+2;
    }

    bool IsEqualName(pwide_t n)
    {
        return name_ == n;
    }

    RegValue()
    {
        memset(&value_,0,sizeof(value_));
        type_ = REG_NONE;
    }

    ~RegValue()
    {
        _Clear();
    }

    void _SetString( pwide_t S, int l = -1 )
    {
        if ( l < 0 )
            l = StrLen(S);
        value_.S.Buffer = (wchar_t*)_MALLOC(l*2+2);
        value_.S.Length = l*2;
        value_.S.MaximumLength = l*2 + 2;
        memcpy(value_.S.Buffer,S,l*2);
        value_.S.Buffer[l] = 0;
    }

    void SetName( pwide_t S, int l = -1 )
    {
        if ( l < 0 )
            l = StrLen(S);
        name_.SetRange(S,l);
    }

    void SetString_REGSZ( pwide_t S, int l = -1 )
    {
        _Clear();
        type_ = REG_SZ;
        _SetString(S,l);
    }

    void Set_REG_SZ( pwide_t S, int l = -1 )
    {
        _Clear();
        type_ = REG_SZ;
        _SetString(S,l);
    }

    void Set_REG_EXPAND_SZ( pwide_t S, int l = -1 )
    {
        _Clear();
        type_ = REG_EXPAND_SZ;
        _SetString(S,l);
    }

    void Set_REG_MULTI_SZ( pwide_t S, int l = -1 )
    {
        _Clear();
        type_ = REG_MULTI_SZ;
        _SetString(S,l);
    }

    void Set_REG_DWORD( unsigned long dw )
    {
        _Clear();
        type_ = REG_DWORD;
        value_.DW = dw;
    }

    void Set_REG_BINARY( void* data, int l )
    {
        _Clear();
        type_ = REG_BINARY;
        value_.B.Buffer = (byte_t*)_MALLOC(l);
        value_.B.Length = l;
        memcpy(value_.B.Buffer,data,l);
    }

    byte_t hex2bin(wchar_t x)
    {
        if ( x >= '0' && x <= '9' ) return byte_t(x-'0');
        if ( x >= 'a' && x <= 'f' ) return byte_t(x-'a');
        __asm int 3;
    }

    void Set_REG_BINARY_STR( pwide_t S, int l )
    {
        _Clear();
        type_ = REG_BINARY;
        value_.B.Buffer = (byte_t*)_MALLOC(l/2);
        value_.B.Length = l/2;
        for ( int i = 0; i < l; i+=2 )
            value_.B.Buffer[i/2] = hex2bin(S[i]) << 4 | hex2bin(S[i+1]);
    }

    void Set(int type,void* data,int datalen)
    {
        if ( type == REG_SZ )
            Set_REG_SZ((wchar_t*)data,datalen/2);
        else if ( type == REG_EXPAND_SZ )
            Set_REG_EXPAND_SZ((wchar_t*)data,datalen/2);
        else if ( type == REG_MULTI_SZ )
            Set_REG_MULTI_SZ((wchar_t*)data,datalen/2);
        else if ( type == REG_DWORD )
            Set_REG_DWORD(*(unsigned long*)data);
        else if ( type == REG_BINARY )
            Set_REG_BINARY(data,datalen);
    }

    u32_t Type() { return type_; }

    void Write(DataStreamPtr& ds, pwide_t filter, unsigned flags, int step=0)
    {
        if ( !(flags&TEGGOVBOX_ONELINE_FORMAT) ) ds->Print("\n");
        if ( !(flags&TEGGOVBOX_ONELINE_FORMAT) )
            for ( int i = 0; i < step; ++i )
                ds->UtfPrint(L"  ");
        ds->UtfPrint(L" \"");
        ds->UtfPrint(+name_);
        ds->UtfPrint(L"\" = ");
        switch ( type_ )
        {
            case REG_SZ:          ds->UtfPrint(L"SZ"); break;
            case REG_EXPAND_SZ:   ds->UtfPrint(L"EZ"); break;
            case REG_MULTI_SZ:    ds->UtfPrint(L"MZ"); break;
            case REG_DWORD:       ds->UtfPrint(L"DW"); break;
            case REG_BINARY:      ds->UtfPrint(L"BY"); break;
        }
        ds->UtfPrint(L":\"");
        if ( type_ == REG_SZ || type_ == REG_EXPAND_SZ || type_ == REG_MULTI_SZ )
            for ( int i=0; i < value_.S.Length/2; ++i )
            {
                int l = 1;
                wchar_t c[4];
                c[0] = value_.S.Buffer[i];
                if ( c[0] == 0 ) { c[0] = L'\\'; c[1] = L'0'; l = 2; }
                else if ( c[0] == L'"' ) { c[0] = L'\\'; c[1] = L'"'; l = 2; }
                else if ( c[0] == L'\\' ) { c[0] = L'\\'; c[1] = L'\\'; l = 2; }
                else if ( c[0] == L'\n' ) { c[0] = L'\\'; c[1] = L'n'; l = 2; }
                ds->UtfWriteText(c,l);
            }
        else if ( type_ == REG_BINARY )
            for ( int i=0; i < value_.B.Length; ++i )
            {
                byte_t c;
                c = value_.B.Buffer[i];
                static wchar_t hextable[] = L"0123456789abcdef";
                wchar_t S[3] = {0,0,0};
                S[0] = hextable[(c>>4)&0x0f];
                S[1] = hextable[c&0x0f];
                ds->UtfWriteText(S,2);
            }
        else if ( type_ == REG_DWORD )
        {
            ds->UtfPrintf(L"%d",value_.DW);
        }
        ds->UtfPrint(L"\"");
    }

    void SetEncoded(BufferT<wchar_t>& data,StringW& type)
    {
        for ( int i = 0; i < data.Count() ; ++i )
            if ( data[i] == '\\' )
            {
                data.Erase(+data+i,1);
                if ( data[i] == '0' ) data[i] = 0;
                else if ( data[i] == 'n' ) data[i] = '\n';
            }
        if ( type == L"SZ" )
            Set_REG_SZ(+data,data.Count());
        else if ( type == L"EZ" )
            Set_REG_EXPAND_SZ(+data,data.Count());
        else if ( type == L"MZ" )
            Set_REG_MULTI_SZ(+data,data.Count());
        else if ( type == L"DW" )
            Set_REG_DWORD(StrAsLong(+StringW(+data,data.Count())));
        else if ( type == L"BY" )
            Set_REG_BINARY_STR(+data,data.Count());
    }
};
typedef rcc_ptr<RegValue> RegValuePtr;

enum RK_STATUS
{
    RK_IS_STATIC,
    RK_IS_VIRTUALIZED,
    RK_IS_MODIFIED,
    RK_IS_PROXY
};

struct RegEntry;
static void REGISTER_PushEntry(RegEntry*);

pwide_t QueryUSID()
{
    static wchar_t usid[256] = {0};
    if ( !usid[0] )
    {
        HANDLE token;
        if ( !OpenProcessToken(GetCurrentProcess(),TOKEN_QUERY,&token) )
            __asm int 3
            ;
        unsigned long foo = 0;
        GetTokenInformation(token,TokenUser,0,0,&foo);
        BufferT<byte_t> tku(foo);
        if ( !GetTokenInformation(token,TokenUser,+tku,foo,&foo) )
            __asm int 3
            ;
        wchar_t* S = 0;
        if ( !ConvertSidToStringSidW(((TOKEN_USER*)+tku)->User.Sid,&S))
            __asm int 3
            ;
        wcscpy(usid,S);
        LocalFree(S);
        for ( wchar_t* p = usid; *p; ++p ) *p = ChrUpper(*p);
    }
    return usid;
}

pwide_t QueryUSIDCLASSES()
{
    static wchar_t usidclasses[256] = {0};
    if ( !usidclasses[0] )
    {
        teggo::GenericWriterT<wchar_t> gw(usidclasses,usidclasses,usidclasses+sizeof(usidclasses)/sizeof(wchar_t)-1);
        (_S*_XOrL("%s_CLASSES",0,0)%QueryUSID()).Write(gw);
    }
    return usidclasses;
}

struct RegEntry : Refcounted
{
    BufferT<RegEntry*>  subkeys_;
    ArrayT<RegValuePtr> values_;
    rcc_ptr<RegEntry>   par_;

    StringW name_;
    RK_STATUS status_;
    bool is_deleted_;
    bool is_not_really_exists_;
    bool trapped_;

    bool IsTrapped() { return trapped_; }

    RegEntry(RegEntry* par,pwide_t S, int l=-1, RK_STATUS status = RK_IS_PROXY, bool trapped=false)
        : par_(Refe(par)), status_(status), is_deleted_(false), is_not_really_exists_(false), trapped_(trapped)
    {
        if ( l < 0 ) l = StrLen(S);
        name_.SetRange(S,l);
        if ( par )
            par->Attach(this);
    }

    ~RegEntry()
    {
        if ( par_ ) par_->Detach(this);
    }

    RegValue* Value(unsigned idx)
    {
        if ( idx < values_.Count() ) return +values_[idx];
        return 0;
    }

    RegValue* Value(pwide_t name)
    {
        if ( RegValue* v = QueryValue(name) )
            return v;
        RegValue* r = new RegValue();
        r->SetName(name);
        values_.Append(RccPtr(r));
        return r;
    }

    void DeleteValue(pwide_t name)
    {
        for ( int i = 0; i < values_.Count(); ++i )
            if ( values_[i]->IsEqualName(name) )
                values_.Erase(+values_+i);
    }

    RegValue* QueryValue(pwide_t name)
    {
        for ( int i = 0; i < values_.Count(); ++i )
            if ( values_[i]->IsEqualName(name) )
                return +values_[i];
        return 0;
    }

    void Detach(RegEntry* r)
    {
        for ( int i = (int)subkeys_.Count()-1; i >= 0 ; --i )
            if ( subkeys_[i] == r )
                subkeys_.Erase(+subkeys_+i);
    }

    void Attach(RegEntry* r)
    {
        subkeys_.Append(r);
    }

    pwide_t Path(BaseStringT<wchar_t>& b = (BaseStringT<wchar_t>&)StringT<wchar_t,128>())
    {
        if ( par_ )
            par_->Path(b);
        if ( name_ )
        {
            b.Append(L'\\');
            b.Append(name_);
        }
        return +b;
    }

    pwide_t Name() { return +name_; }
    long    NameLength() { return name_.Length(); }

    RegEntry* Subkey(int i)
    {
        if ( i >= 0 && i < subkeys_.Count() )
            return subkeys_[i];
        else
        {
            XLOG|_S* _XOr("<ERR> invalid subkey index %d of key '%s' ['apif_register.cpp':%d]",67,153029431)
            %i
            %Path()
            %__LINE__;
            //__asm int 3;
            return 0;
        }
    }

    int GetMaxValueLen() { return 1024; }
    int GetValuesCount() { return values_.Count(); }

    int SubkeysCount() { return subkeys_.Count(); }

    int GetOwnedSubkeysCount()
    {
        int n = 0;
        for ( int i = 0; i < subkeys_.Count(); ++i )
            if ( subkeys_[i]->is_not_really_exists_ )
                ++n;
        return n;
    }

    int GetNormalSubkeysCount()
    {
        int n = 0;
        for ( int i = 0; i < subkeys_.Count(); ++i )
            if ( !subkeys_[i]->IsProxy() && !subkeys_[i]->is_deleted_ )
                ++n;
        return n;
    }

    int GetProxedSubkeysCount()
    {
        int n = 0;
        for ( int i = 0; i < subkeys_.Count(); ++i )
            if ( subkeys_[i]->IsProxy() )
                ++n;
        return n;
    }

    bool HasEqualName(pwide_t S, int l = -1)
    {
        if ( l < 0 ) l = StrLen(S);
        if ( l != name_.Length() ) return false;
        for ( int i = 0; i < l ; ++i)
            if ( S[i] != (+name_)[i] )
                return false;
        return true;
    }

    RegEntry* Search(pwide_t S, int l = -1)
    {
        if ( l < 0 ) l = StrLen(S);
        if ( l == 11 && memcmp(S,L"WOW6432NODE",22) == 0 )
            return this;
        for ( int i = 0; i < subkeys_.Count(); ++i )
            if ( subkeys_[i]->HasEqualName(S,l) )
                return subkeys_[i];
        return 0;
    }

    void _Clear()
    {
        for ( int i = 0; i < subkeys_.Count(); ++i )
            subkeys_[i]->_Clear();
        subkeys_.Clear();
    }

    void Renew()
    {
        is_deleted_ = false;
        if ( status_ == RK_IS_PROXY )
            REGISTER_PushEntry(this);
        status_ = RK_IS_MODIFIED;
        if ( par_ )
            par_->NotifyRenew(this);
    }

    bool IsDeleted() { return is_deleted_; }
    bool IsValid() { return this != 0; }
    bool IsProxy() { return status_ == RK_IS_PROXY; }

    void Delete()
    {
        if ( status_ == RK_IS_PROXY )
            REGISTER_PushEntry(this);
        status_ = RK_IS_MODIFIED;
        is_deleted_ = true;
        _Clear();
        if ( par_ )
            par_->NotifyDelete(this);
    }

    void NotifyDelete(RegEntry* r)
    {
        RegEntry** re = 0;
        RegEntry** self = 0;
        for ( int i = 0; i < subkeys_.Count(); ++i )
        {
            if ( subkeys_[i] == r ) self = &subkeys_[i];
            else if ( !subkeys_[i]->IsProxy() && !subkeys_[i]->IsDeleted() ) re = &subkeys_[i];
        }

        if ( self && re && self != re )
            cxx_swap(*self,*re);

        if ( !is_deleted_ )
            if ( status_ == RK_IS_PROXY )
            {
                REGISTER_PushEntry(this);
                status_ = RK_IS_MODIFIED;
            }
    };

    void NotifyRenew(RegEntry* r)
    {
        RegEntry** re = 0;
        RegEntry** self = 0;
        for ( int i = subkeys_.Count()-1; i>=0; --i )
        {
            if ( subkeys_[i] == r ) self = &subkeys_[i];
            else if ( subkeys_[i]->IsProxy() || subkeys_[i]->IsDeleted() ) re = &subkeys_[i];
        }

        if ( self && re && self != re )
            cxx_swap(*self,*re);
    };

    void Write(DataStreamPtr& ds, pwide_t filter, unsigned flags, int step=0)
    {
        if ( par_ )
        {
            if ( !(flags&TEGGOVBOX_ONELINE_FORMAT) ) ds->Print("\n");
            if ( !(flags&TEGGOVBOX_ONELINE_FORMAT) )
                for ( int i = 0; i < step; ++i )
                    ds->UtfPrint(L"  ");
            ds->UtfPrint(L"(");
            if ( status_ == RK_IS_PROXY ) ds->UtfPrint(L"@");
            ds->UtfPrint(L"\"");
            if ( name_ == QueryUSIDCLASSES() )
                ds->UtfPrint(L"{%USIDCLASSES%}");
            else if ( name_ == QueryUSID() )
                ds->UtfPrint(L"{%USID%}");
            else
                ds->UtfPrint(+name_);
            ds->UtfPrint(L"\"");
        }
        for ( int i = 0; i < values_.Count(); ++i )
            values_[i]->Write(ds,filter,flags,step+1);
        for ( int i = 0; i < subkeys_.Count(); ++i )
            subkeys_[i]->Write(ds,filter,flags,step+1);
        if ( par_ )
        {
            //for ( int i = 0; i < step; ++i ) ds->UtfPrint(L"  ");
            ds->UtfPrint(L")");
        }
    }
};
typedef rcc_ptr<RegEntry> RegEntryPtr;

struct _REGISTER
{
    Tlocker l_;
    RegEntry*            root_;
    RegEntry*            USER;
    ArrayT<RegEntryPtr> stored_;
    ArrayT<StringW>     exceptions_;

    void _PushEntry(RegEntry* e)
    {
        stored_.Push(RccRefe(e));
    }

    ArrayT<RegEntryPtr>& Stored() { return stored_; }

    _REGISTER()
    {
        filtered_ = false;
        virtual_ = false;
        enabled_ = true;
        root_ = new RegEntry(0,L"",0,RK_IS_STATIC);
        stored_.Push(RccPtr(root_));

        RegEntry* e, *e1;
        stored_.Push(RccPtr(e = new RegEntry(root_,L"REGISTRY",-1,RK_IS_STATIC)));
        stored_.Push(RccPtr(e1 = new RegEntry(e,L"USER",-1,RK_IS_STATIC)));
        USER = e1;

        stored_.Push(RccPtr(new RegEntry(e1,QueryUSIDCLASSES(),-1,RK_IS_STATIC,true)));
        stored_.Push(RccPtr(e1 = new RegEntry(e1,QueryUSID(),-1,RK_IS_STATIC)));
        stored_.Push(RccPtr(new RegEntry(e1,L"SOFTWARE",-1,RK_IS_STATIC)));

        stored_.Push(RccPtr(e = new RegEntry(e,L"MACHINE",-1,RK_IS_STATIC)));
        stored_.Push(RccPtr(e = new RegEntry(e,L"SOFTWARE",-1,RK_IS_STATIC)));
        stored_.Push(RccPtr(e = new RegEntry(e,L"CLASSES",-1,RK_IS_STATIC,true)));
    }

    ~_REGISTER()
    {
        root_->_Clear();
        stored_.Clear();
    }

    bool CXX_NO_INLINE IsException(pwide_t path)
    {
        return false;
    }

    bool CXX_NO_INLINE _ExistsInRegestry(RegEntry* e, pwide_t S, int l = -1 )
    {
        if ( l < 0 ) l = StrLen(S);
        StringW rpath; e->Path(rpath); rpath.Append(L'\\').AppendRange(S,l);

        OBJECT_ATTRIBUTES oa;
        UNICODE_STRING us = { rpath.Length()*2, rpath.Length()*2+2, (wchar_t*)+rpath };
        InitializeObjectAttributes(&oa,&us,OBJ_CASE_INSENSITIVE,0,0);
        HANDLE key = 0;

        if ( long st = NtOpenKey(&key,GENERIC_READ,&oa) )
            return false;
        else
            NtClose(key);

        return true;
    }

    RegEntryPtr CXX_NO_INLINE _GetEntry(RegEntry* par,pwide_t path, bool create_if_notexists, bool* created = 0, bool* deleted = 0,
                                        bool gen_proxy = true)
    {
        StringW b = path;
        b.ToUpper();
        if ( created ) *created = false;
        if ( deleted ) *deleted = false;
        if ( IsException(+b) ) return RegEntryPtr(0);

        RegEntryPtr foo;
        RegEntry* e = par?par:+root_;
        pwide_t p = +b;
        while ( *p )
        {
            while ( *p == L'\\' ) ++p;
            pwide_t pE = p; // +1;
            while ( *pE && *pE != '\\' ) ++pE;
            if ( RegEntry* ee = e->Search(p,pE-p) )
                e = ee;
            else
            {
                if ( !e->IsTrapped() && IsFiltered() && !gen_proxy ) return RegEntryPtr(0);
                bool exists = _ExistsInRegestry(e,p,pE-p);
                if ( create_if_notexists || exists )
                {

                    //if ( !exists )
                    //  __asm int 3
                    //  ;

                    RK_STATUS status;
                    if ( IsFiltered() )
                        status = !exists&&create_if_notexists?RK_IS_MODIFIED:RK_IS_PROXY;
                    else
                        status = create_if_notexists?RK_IS_MODIFIED:RK_IS_PROXY;
                    e = new RegEntry(e,p,pE-p,status,e->IsTrapped());
                    if ( !exists ) e->is_not_really_exists_ = true;
                    if ( !e->IsProxy() ) _PushEntry(e);
                    foo = RccPtr(e);
                    if ( created ) *created = !exists;
                }
                else
                    return RegEntryPtr(0);
            }
            if ( e->IsDeleted() )
            {
                if ( !create_if_notexists )
                {
                    if ( deleted ) *deleted = true;
                    return RegEntryPtr(0);
                }
                else
                {
                    if ( created ) *created = true;
                    e->Renew();
                }
            }
            p = pE;
        }

        return RccRefe(e);
    }

    RegEntryPtr GetOrCreateEntry(RegEntry* par,pwide_t path, bool* created = 0)
    {
        return _GetEntry(par,path,true,created,0);
    }

    RegEntryPtr GetOrCreateEntryFiltered(RegEntry* par,pwide_t path, bool* created = 0)
    {
        return _GetEntry(par,path,true,created,0,false);
    }

    RegEntryPtr GetEntry(RegEntry* par,pwide_t path,bool* deleted = 0)
    {
        return _GetEntry(par,path,false,0,deleted);
    }

    RegEntryPtr GetEntryFiltered(RegEntry* par,pwide_t path,bool* deleted = 0)
    {
        return _GetEntry(par,path,false,0,deleted,false);
    }

    CRITICAL_SECTION* _GetLock() { return l_; }

    void Write(DataStreamPtr& ds, pwide_t filter, unsigned flags)
    {
        //ds->UtfPrint(L"(REGISTRY FILE)");
        root_->Write(ds,filter,flags);
    }

    void SkipUntil(wchar_t chr,wchar_t*& p,wchar_t* pE)
    {
        for ( ; p != pE && *p != chr ; ++p )
            if ( *p == '\\' ) ++p;
    }

    void Load_(RegEntry* par,wchar_t*& p, wchar_t* pE)
    {
        //__asm int 3;
        SkipUntil('(',p,pE);
        if ( *p != '(' ) return;
        SkipUntil('"',p,pE);
        wchar_t* nS = ++p;
        SkipUntil('"',p,pE);
        StringW key_name(nS,p-nS);
        ++p;
        RegEntryPtr cur = _GetEntry(+par,+key_name,true);
        while ( p != pE && *p != '('  && *p != ')' )
        {
            if ( *p == ' ' ) { ++p; continue; }
            if ( *p == '"' )
            {
                nS = ++p;
                SkipUntil('"',p,pE);
                StringW val_name(nS,p-nS);
                SkipUntil('=',p,pE); ++p;
                while ( *p == ' ' ) ++p;
                nS = p;
                SkipUntil(':',p,pE);
                StringW val_type(nS,p-nS);
                SkipUntil('"',p,pE);
                nS = ++p;
                SkipUntil('"',p,pE);
                BufferT<wchar_t> val_data(nS,p);
                ++p;
                cur->Renew();
                cur->Value(+val_name)->SetEncoded(val_data,val_type);
            }
            else
                // opps
                ++p;
        }
        while ( p != pE && *p != ')' )
        {
            if ( *p == ' ' ) { ++p; continue; }
            if ( *p == '(' )
            {
                Load_(+cur,p,pE);
            }
            else
                //opps
                ++p;
        }
        if ( p != pE && *p == ')' ) ++p;
    }

    void Load(DataStreamPtr& ds)
    {
        StringW appfolder = Hinstance->GetMainModuleDir();
        appfolder.Replace(L"/",'\\');
        appfolder.EscMask();
        BufferT<wchar_t> regS;
        StringW MFOLDER = _XOrL("{%MAINFOLDER%}",0,0);
        StringW USICLSS = _XOrL("{%USIDCLASSES%}",0,0);
        StringW USIDCLASSES = QueryUSIDCLASSES();
        StringW USIDS = _XOrL("{%USID%}",0,0);
        StringW USID = QueryUSID();
        while ( !ds->Error() )
        {
            regS.Clear();
            ds->UtfReadLineEx(regS);
            regS.Push(0);

            if ( regS.Count() > 2 )
            {
                regS.Replace(+MFOLDER,MFOLDER.Length(),+appfolder,appfolder.Length());
                regS.Replace(+USICLSS,USICLSS.Length(),+USIDCLASSES,USIDCLASSES.Length());
                regS.Replace(+USIDS,USIDS.Length(),+USID,USID.Length());
                wchar_t* p = regS.Begin();
                if ( logrgloading_ )
                    XDBG | _S* _XOr("RGLOAD: %s",11,161745788) %+regS;

                //if ( DataStreamPtr ds = OpenStream("registry.txt",STREAM_WRITE+STREAM_CREATE) )
                //  ds->UtfWriteText(+regS,regS.Count());

                Load_(root_,p,regS.End()-1);
            }
        }
    }

    bool filtered_;
    bool virtual_;
    bool enabled_;

    void SetFilteredMode(bool val) { filtered_ = val; }
    bool IsFiltered() { return filtered_; }
    void SetVirtualMode(bool val) { virtual_ = val; }
    bool IsVirtual() { return virtual_; }

    bool IsEnabled() { return enabled_; }
    void Enable() { enabled_ = true; }
    void Disable() { enabled_ = false; }

    bool logrgloading_;
    void LogRgLoading(bool val) { logrgloading_ = val; }

};
_REGISTER g_register = _REGISTER();

struct
{
    _REGISTER* operator->() const { return &g_register; }
    operator CRITICAL_SECTION* () const { return (*this)->_GetLock(); }
} const REGISTER = {};

static void REGISTER_PushEntry(RegEntry* e) { REGISTER->_PushEntry(e); }

inline void Delete(RegEntry* p)
{
    p->Release();
}

typedef HandledObjectsArray<RegEntry,u32_t> HOA;

struct RegObjectFilter : APIfilter
{

    HOA handles_;
    RegValuePtr fake_valueS_;

    RegObjectFilter()
    {
        memset(rkh_recs_,0,sizeof(rkh_recs_));
        fake_valueS_ = RccPtr(new RegValue());
        fake_valueS_->SetString_REGSZ(L"");
    };

    APIF_STATUS DoCreateHkey(void* _a, unsigned* result)
    {
        NT_CREATE_HKEY_ARGS* args = (NT_CREATE_HKEY_ARGS*)_a;
        StringW path(args->ObjectAttributes->ObjectName->Buffer,args->ObjectAttributes->ObjectName->Length/2);
        XDBG|_S* _XOr(".apif.CreateHkey %08x/'%s'",27,162073449) %args->ObjectAttributes->RootDirectory %path;
        __lockon__(REGISTER)
        {
            RegEntryPtr par;
            if ( args->ObjectAttributes->RootDirectory )
                if ( !(par=QueryPar(args->ObjectAttributes->RootDirectory)) )
                    return APIF_CONTINUE;
            bool created = false;
            if ( RegEntryPtr p = REGISTER->GetOrCreateEntryFiltered(+par,+path,&created) )
            {
                *args->KeyHandle = handles_.Put(Refe(+p));
                *result = 0;
                XLOG|_S* _XOr("<APIF/RGS> create hkey '%s' -> %08x, created: %?",49,159910536)
                %p->Path() %*args->KeyHandle %created;
                if ( args->Disposition )
                    *args->Disposition = created?REG_CREATED_NEW_KEY:REG_OPENED_EXISTING_KEY;
                return APIF_RETURN;
            }
            else if ( REGISTER->IsVirtual() )
            {
                *result = STATUS_OBJECT_NAME_NOT_FOUND;
                return APIF_RETURN;
            }
            else
            {
                if ( handles_._Get(args->ObjectAttributes->RootDirectory) )
                    if ( HANDLE rootdir = _LookForRegKeyHandle(+par,0) )
                    {
                        OBJECT_ATTRIBUTES oa = *args->ObjectAttributes;
                        oa.RootDirectory = rootdir;
                        *result = NtCreateKey(args->KeyHandle,args->DesiredAccess,&oa,args->TitleIndex,args->Class,args->CreateOptions,
                                              args->Disposition);
                        return APIF_RETURN;
                    }
                    else
                    {
                        *result = STATUS_OBJECT_NAME_NOT_FOUND;
                        return APIF_RETURN;
                    }
                return APIF_CONTINUE;
            }
        }
        //return APIF_CONTINUE;
    }


    APIF_STATUS DoCreateHkey_1(void* _a, unsigned* result)
    {
        NT_CREATE_HKEY_ARGS* args = (NT_CREATE_HKEY_ARGS*)_a;
        StringW path(args->ObjectAttributes->ObjectName->Buffer,args->ObjectAttributes->ObjectName->Length/2);
        XDBG|_S* _XOr(".apif.CreateHkey (unvirtualized) %08x/'%s'",43,167447259) %args->ObjectAttributes->RootDirectory %path;
        __lockon__(REGISTER)
        {
            RegEntryPtr par;
            if ( args->ObjectAttributes->RootDirectory )
                if ( !(par=QueryPar(args->ObjectAttributes->RootDirectory)) )
                    return APIF_CONTINUE;
            if ( handles_._Get(args->ObjectAttributes->RootDirectory) )
                if ( HANDLE rootdir = _LookForRegKeyHandle(+par,0) )
                {
                    OBJECT_ATTRIBUTES oa = *args->ObjectAttributes;
                    oa.RootDirectory = rootdir;
                    *result = NtCreateKey(args->KeyHandle,args->DesiredAccess,&oa,args->TitleIndex,args->Class,args->CreateOptions,
                                          args->Disposition);
                    return APIF_RETURN;
                }
                else
                {
                    *result = STATUS_OBJECT_NAME_NOT_FOUND;
                    return APIF_RETURN;
                }
            return APIF_CONTINUE;
        }
    }

    APIF_STATUS DoOpenHkey(void* _a, unsigned* result)
    {
        NT_OPEN_HKEY_ARGS* args = (NT_OPEN_HKEY_ARGS*)_a;
        StringW path(args->ObjectAttributes->ObjectName->Buffer,args->ObjectAttributes->ObjectName->Length/2);
        XDBG|_S* _XOr(".apif.OpenHkey %08x/'%s'",25,170461929) %args->ObjectAttributes->RootDirectory %path;
        bool deleted = false;
        __lockon__(REGISTER)
        {
            RegEntryPtr par;
            if ( args->ObjectAttributes->RootDirectory )
                if ( !(par=QueryPar(args->ObjectAttributes->RootDirectory)) )
                    return APIF_CONTINUE;
            if ( RegEntryPtr p = REGISTER->GetEntryFiltered(+par,+path,&deleted) )
            {
                *args->KeyHandle = handles_.Put(Refe(+p));
                *result = 0;
                XLOG|_S* _XOr("<APIF/RGS> open hkey '%s' -> %08x:%s",37,168692230)
                %p->Path() %*args->KeyHandle %(p->IsProxy()?":proxy":"");
                return APIF_RETURN;
            }
            else if (deleted)
            {
                *result = STATUS_OBJECT_NAME_NOT_FOUND;
                XLOG|_S* _XOr("<APIF/RGS> couldn't open hkey, the hkey is deleted '%s%s%s'",60,175573551)
                %(par?par->Path():L"")%(par?L"\\":L"")%path;
                return APIF_RETURN;
            }
            else if ( REGISTER->IsVirtual() )
            {
                *result = STATUS_OBJECT_NAME_NOT_FOUND;
                return APIF_RETURN;
            }
            else
            {
                XLOG|_S* _XOr("<APIF/RGS> open native hkey '%08x:%s'",38,179636845)
                %args->ObjectAttributes->RootDirectory %path;
                if ( handles_._Get(args->ObjectAttributes->RootDirectory) )
                    if ( HANDLE rootdir = _LookForRegKeyHandle(+par,0) )
                    {
                        OBJECT_ATTRIBUTES oa = *args->ObjectAttributes;
                        oa.RootDirectory = rootdir;
                        *result = NtOpenKey(args->KeyHandle,args->DesiredAccess,&oa);
                        return APIF_RETURN;
                    }
                    else
                    {
                        *result = STATUS_OBJECT_NAME_NOT_FOUND;
                        return APIF_RETURN;
                    }
                return APIF_CONTINUE;
            }
        }
    }

    APIF_STATUS DoOpenHkey_1(void* _a, unsigned* result)
    {
        NT_OPEN_HKEY_ARGS* args = (NT_OPEN_HKEY_ARGS*)_a;
        StringW path(args->ObjectAttributes->ObjectName->Buffer,args->ObjectAttributes->ObjectName->Length/2);
        XDBG|_S* _XOr(".apif.OpenHkey (unvirtualized) %08x/'%s'",41,178064821) %args->ObjectAttributes->RootDirectory %path;
        bool deleted = false;
        __lockon__(REGISTER)
        {
            RegEntryPtr par;
            if ( args->ObjectAttributes->RootDirectory )
                if ( !(par=QueryPar(args->ObjectAttributes->RootDirectory)) )
                    return APIF_CONTINUE;
            XLOG|_S* _XOr("<APIF/RGS> open native hkey '%08x:%s'",38,181145028)
            %args->ObjectAttributes->RootDirectory %path;
            if ( handles_._Get(args->ObjectAttributes->RootDirectory) )
                if ( HANDLE rootdir = _LookForRegKeyHandle(+par,0) )
                {
                    OBJECT_ATTRIBUTES oa = *args->ObjectAttributes;
                    oa.RootDirectory = rootdir;
                    *result = NtOpenKey(args->KeyHandle,args->DesiredAccess,&oa);
                    return APIF_RETURN;
                }
                else
                {
                    *result = STATUS_OBJECT_NAME_NOT_FOUND;
                    return APIF_RETURN;
                }
            return APIF_CONTINUE;
        }
    }

    RegEntryPtr QueryPar(HANDLE h)
    {
        if ( RegEntryPtr par = RccRefe(GetProxy(h) ) )
            return par;

        BufferT<byte_t> m(sizeof(OBJECT_NAME_INFORMATION)+1024,0);
        OBJECT_NAME_INFORMATION* kbi = (OBJECT_NAME_INFORMATION*)+m;
        kbi->ObjectName.Buffer = (wchar_t*)(kbi+1);
        kbi->ObjectName.Length = 0;
        kbi->ObjectName.MaximumLength = 1024;
        unsigned long result = 0;

        if ( long st = NtQueryObject(h,MyObjectNameInformation,kbi,m.Count(),&result) )
            return RegEntryPtr(0);
        return REGISTER->GetEntryFiltered(0,kbi->ObjectName.Buffer,0);
    }

    RegEntry* GetProxy(HANDLE h)
    {
        if ( HOA::R o = handles_.Get(h) )
            return +o;
        else
            return 0;
    }

    APIF_STATUS DoDuplicateKey(void* _a, unsigned* result)
    {
        NT_DUPLICATE_ARGS* args = (NT_DUPLICATE_ARGS*)_a;
        XDBG|_S* _XOr(".apif.DuplicateHkey %08x",25,188026349) %args->SourceHandle;
        if ( HOA::R o = handles_.Get(args->SourceHandle) )
        {
            __lockon__(REGISTER)
            if ( o->IsValid() )
            {
                *args->TargetHandle = handles_.Put(Refe(+o));
                XLOG|_S* _XOr("<APIF/RGS> duplicate hkey (%08x) '%s' -> %08x",46,185273611)
                %args->SourceHandle
                %o->Path()
                %*args->TargetHandle;
                *result = 0;
            }
            else
                *result = STATUS_INVALID_HANDLE;
            return APIF_RETURN;
        }
        return APIF_CONTINUE;
    };

    APIF_STATUS DoCloseHkey(void* _a, unsigned* result)
    {
        NT_CLOSE_ARGS* args = (NT_CLOSE_ARGS*)_a;
        XDBG|_S* _XOr(".apif.CloseHkey %08x",21,189140318) % args->FileHandle;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
        {
            __lockon__(REGISTER)
            {
                if ( o->IsValid() )
                {
                    XLOG|_S* _XOr("<APIF/RGS> close hkey (%08x) '%s'",34,190647669)
                    %args->FileHandle
                    %o->Path();
                    _ClearRegKeyHandle(+o);
                    handles_.Close(args->FileHandle);
                    *result = 0;
                }
                else
                { *result = STATUS_INVALID_HANDLE; }
            }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }

    APIF_STATUS DoDeleteHkey(void* _a, unsigned* result)
    {
        NT_DELKEY_ARGS* args = (NT_DELKEY_ARGS*)_a;
        XDBG|_S* _XOr(".apif.DeleteHkey %08x",22,193268893) % args->KeyHandle;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
        {
            //if (o)
            __lockon__(REGISTER)
            {
                if ( o->IsValid() )
                {
                    XLOG|_S* _XOr("<APIF/RGS> delete hkey (%08x) '%s'",35,194841781)
                    %args->KeyHandle
                    %o->Path();
                    o->Delete();
                    _ClearRegKeyHandle(+o);
                    handles_.Close(args->KeyHandle);
                    *result = 0;
                }
                else
                { *result = STATUS_INVALID_HANDLE; }
            }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }

    void CloneNativeKey(HOA::R& o, HANDLE KeyHandle)
    {
        o->Renew();
        int idx = 0;
        long st;
        BufferT<byte_t> b(2*1024);
        if ( HANDLE hk = _LookForRegKeyHandle(+o,KeyHandle))
            for (;; ++idx)
            {
                unsigned long r;
                KEY_VALUE_FULL_INFORMATION* ki = (KEY_VALUE_FULL_INFORMATION*)+b;
                st = NtEnumerateValueKey(hk,idx,KeyValueFullInformation,ki,b.Count(),&r);
                if ( st == STATUS_NO_MORE_ENTRIES )
                    break;
                else if ( st )
                {
                    XLOG|_S* _XOr("<ERR> failed(%08x) to query key value %d of '%s'",49,197528798)
                    %st %idx %o->Path();
                    //__asm int 3;
                    continue;
                }
                o->Value(+StringT<wchar_t,32>(ki->Name,ki->NameLength/2).ToUpper())->Set(ki->Type,+b+ki->DataOffset,ki->DataLength);
            }
    }

    APIF_STATUS DoDeleteHkeyVal(void* _a, unsigned* result)
    {
        NT_DELKEYVAL_ARGS* args = (NT_DELKEYVAL_ARGS*)_a;
        StringW valname(args->ValueName->Buffer,args->ValueName->Length/2);
        XDBG|_S* _XOr(".apif.DeleteHkeyVal %08x %s",28,204672019) % args->KeyHandle % valname;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
        {
            __lockon__(REGISTER)
            {
                if ( o->IsValid() )
                {
                    XLOG|_S* _XOr("<APIF/RGS> delete hkey value (%08x) '%s'/'%s'",46,202443825)
                    %args->KeyHandle
                    %o->Path()
                    %valname;
                    if ( o->IsProxy() && REGISTER->IsFiltered() )
                        if ( HANDLE hk = _LookForRegKeyHandle(+o,args->KeyHandle,1) )
                        {
                            XLOG|_S* _XOr("<APIF/RGS> delvalue '%s' redirecting to native registry",56,205655106) %valname;
                            args->KeyHandle = hk;
                            return APIF_CONTINUE;
                        }

                    valname.ToUpper();

                    if ( o->IsProxy() )
                        CloneNativeKey(o,args->KeyHandle);

                    *result = 0;
                    o->DeleteValue(+valname);
                }
                else
                    *result = STATUS_INVALID_HANDLE;
            }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }

    APIF_STATUS DoSetValHkey(void* _a, unsigned* result)
    {
        NT_SETVAL_HKEY_ARGS* args = (NT_SETVAL_HKEY_ARGS*)_a;
        //__asm int 3;
        StringW valname(args->ValueName->Buffer,args->ValueName->Length/2);
        XDBG|_S* _XOr(".apif.SetValHkey %08x, val '%s', type %02x, size %d",52,210046877)
        % args->KeyHandle
        % valname
        % args->Type
        % args->DataSize;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
        {
            __lockon__(REGISTER)
            {
                if ( o->IsValid() )
                {
                    if ( o->IsProxy() && REGISTER->IsFiltered() )
                        if ( HANDLE hk = _LookForRegKeyHandle(+o,args->KeyHandle,1) )
                        {
                            XLOG|_S* _XOr("<APIF/RGS> setvalue '%s' redirecting to native registry",56,218042327) %valname;
                            args->KeyHandle = hk;
                            return APIF_CONTINUE;
                        }
                    XDBG|_S* _XOr("<APIF/RGS> proxy: %d, filtered: %d",35,222105365) %o->IsProxy() %REGISTER->IsFiltered();
                    valname.ToUpper();
                    XLOG|_S* _XOr("<APIF/RGS> set value '%s'=%s:'%s' of hkey(%08x) '%s'",53,219811642)
                    %valname
                    %(args->Type==REG_DWORD?"dword"
                      :args->Type==REG_SZ?"string"
                      :args->Type==REG_BINARY?"bin"
                      :0|_S*"%02x"%args->Type)
                    %(args->Type==REG_DWORD?(0|_S*L"%d"%*(int*)args->Data)
                      :args->Type==REG_SZ?(pwide_t)args->Data
                      :L"???")
                    %args->KeyHandle
                    %o->Path();

                    if ( o->IsProxy() )
                        CloneNativeKey(o,args->KeyHandle);

                    *result = 0;
                    o->Value(+valname)->Set(args->Type,args->Data,args->DataSize);
                }
                else
                    *result = STATUS_INVALID_HANDLE;
            }
            //else
            //  { *result = STATUS_INVALID_HANDLE; }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }


    APIF_STATUS DoQueryHkey(void* _a, unsigned* result)
    {
        NT_QUERYKEY_HKEY_ARGS* args = (NT_QUERYKEY_HKEY_ARGS*)_a;
        //__asm int 3;
        XDBG|_S* _XOr(".apif.QueryHkey %08x",21,224137076) % args->KeyHandle;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
        {
            //if (!o)
            //  { *result = STATUS_INVALID_HANDLE; }
            //else
            __lockon__(REGISTER)
            {
                if ( !o->IsValid() )
                { *result = STATUS_INVALID_HANDLE; }
                else
                {
                    XLOG|_S* _XOr("<APIF/RGS> query hkey(%08x) '%s'",33,230100847)
                    %args->KeyHandle
                    %o->Path();
                    if ( o->IsProxy() )
                    {
                        HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle);
                        if ( kh )
                        {
                            args->KeyHandle = kh;
                            //XDBG| _S*"<APIF/RGS> processing proxy %08x" %kh;
                            return APIF_CONTINUE;
                        }
                        else
                        {
                            //__asm int 3;
                            *result = STATUS_INVALID_HANDLE;
                        }
                    }
                    else if ( args->KeyInformationClass == KeyNameInformation )
                    {
                        KEY_NAME_INFORMATION* ki = (KEY_NAME_INFORMATION*)args->KeyInformation;
                        StringW rpath; o->Path(rpath);
                        *args->ResultLength = rpath.Length()*2+2 + 4;
                        if (sizeof(ki) > args->Length) { *result = STATUS_BUFFER_TOO_SMALL; return APIF_RETURN; }
                        ki->NameLength = rpath.Length()*2;
                        if (*args->ResultLength > args->Length)
                        {
                            *result = STATUS_BUFFER_OVERFLOW;
                            return APIF_RETURN;
                        }
                        memcpy(ki->Name,+rpath,ki->NameLength+2);
                    }
                    else if ( args->KeyInformationClass == KeyBasicInformation )
                    {
                        KEY_BASIC_INFORMATION* ki = (KEY_BASIC_INFORMATION*)args->KeyInformation;
                        StringW rpath; o->Path(rpath);
                        *args->ResultLength = rpath.Length()*2+2 + 4 + 4 + 8;
                        if (sizeof(*ki) > args->Length) { *result = STATUS_BUFFER_TOO_SMALL; return APIF_RETURN; }
                        ki->NameLength = rpath.Length()*2;
                        memset(&ki->LastWriteTime,0,sizeof(ki->LastWriteTime));
                        if (*args->ResultLength > args->Length) { *result = STATUS_BUFFER_OVERFLOW; return APIF_RETURN; }
                        memcpy(ki->Name,+rpath,ki->NameLength+2);
                    }
                    else if ( args->KeyInformationClass == KeyCachedInformation )
                    {
                        KEY_CACHED_INFORMATION* ki = (KEY_CACHED_INFORMATION*)args->KeyInformation;
                        StringW rpath; o->Path(rpath);
                        *args->ResultLength = rpath.Length()*2+sizeof(*ki);
                        if (sizeof(*ki) > args->Length) { *result = STATUS_BUFFER_TOO_SMALL; return APIF_RETURN; }
                        if ( HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle) )
                        {
                            *result = NtQueryKey(kh,args->KeyInformationClass,args->KeyInformation,args->Length,args->ResultLength);
                            if ( *result && *result != STATUS_BUFFER_OVERFLOW )
                                return APIF_RETURN;
                            ki->SubKeys += o->GetNormalSubkeysCount();
                        }
                        else
                        {
                            ki->NameLength = rpath.Length()*2;
                            memset(&ki->LastWriteTime,0,sizeof(ki->LastWriteTime));
                            if (*args->ResultLength > args->Length) { *result = STATUS_BUFFER_OVERFLOW; return APIF_RETURN; }
                            memcpy(ki->Name,+rpath,ki->NameLength+2);
                            *result = 0;
                            ki->MaxValueDataLen = 0;
                            ki->SubKeys = o->GetNormalSubkeysCount();
                        }
                        ki->MaxNameLen = 256;
                        ki->MaxValueNameLen = 256;
                        if ( !o->IsProxy() )
                            ki->MaxValueDataLen = cxx_max<unsigned>(o->GetMaxValueLen(),ki->MaxValueDataLen);
                        if ( !o->IsProxy() )
                            ki->Values = o->GetValuesCount();
                        return APIF_RETURN;
                    }
                    else if ( args->KeyInformationClass == KeyHandleTagsInformation )
                    {
                        KEY_HANDLE_TAGS_INFORMATION* ki = (KEY_HANDLE_TAGS_INFORMATION*)args->KeyInformation;
                        *args->ResultLength = sizeof(*ki);
                        if (sizeof(*ki) > args->Length) { *result = STATUS_BUFFER_TOO_SMALL; return APIF_RETURN; }
                        ki->HandleTags = 0;
                        return APIF_RETURN;
                    }
                    else if ( args->KeyInformationClass == KeyVirtualizationInformation )
                    {
                        KEY_SET_VIRTUALIZATION_INFORMATION* ki = (KEY_SET_VIRTUALIZATION_INFORMATION*)args->KeyInformation;
                        *args->ResultLength = sizeof(*ki);
                        if (sizeof(*ki) > args->Length) { *result = STATUS_BUFFER_TOO_SMALL; return APIF_RETURN; }
                        memset(ki,0,sizeof(*ki));
                        return APIF_RETURN;
                    }
                    else if ( args->KeyInformationClass == KeyFlagsInformation )
                    {
                        KEY_CONTROL_FLAGS_INFORMATION* ki = (KEY_CONTROL_FLAGS_INFORMATION*)args->KeyInformation;
                        *args->ResultLength = sizeof(*ki);
                        if (sizeof(*ki) > args->Length) { *result = STATUS_BUFFER_TOO_SMALL; return APIF_RETURN; }
                        ki->ControlFlags = 0;
                        return APIF_RETURN;
                    }
                    else if ( args->KeyInformationClass == KeyFullInformation )
                    {
                        KEY_FULL_INFORMATION* ki = (KEY_FULL_INFORMATION*)args->KeyInformation;
                        memset(ki,0,sizeof(*ki));
                        StringW rpath; o->Path(rpath);
                        if (sizeof(*ki) > args->Length) { *result = STATUS_BUFFER_TOO_SMALL; return APIF_RETURN; }
                        if ( HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle) )
                        {
                            *result = NtQueryKey(kh,args->KeyInformationClass,args->KeyInformation,args->Length,args->ResultLength);
                            if ( *result && *result != STATUS_BUFFER_OVERFLOW )
                                return APIF_RETURN;
                            ki->SubKeys += o->GetNormalSubkeysCount();
                        }
                        else
                        {
                            *args->ResultLength = sizeof(*ki);
                            memset(&ki->LastWriteTime,0,sizeof(ki->LastWriteTime));
                            ki->MaxValueDataLen = 0;
                            ki->SubKeys = o->GetNormalSubkeysCount();
                        }
                        ki->MaxNameLen = 256;
                        ki->MaxValueNameLen = 256;
                        if ( !o->IsProxy() )
                            ki->MaxValueDataLen = cxx_max<unsigned>(o->GetMaxValueLen(),ki->MaxValueDataLen);
                        if ( !o->IsProxy() )
                            ki->Values = o->GetValuesCount();
                        return APIF_RETURN;
                    }
                    else
                    {
                        __asm int 3;
                        *result = STATUS_INVALID_PARAMETER;
                        return APIF_RETURN;
                    }
                    *result = 0;
                }
            }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }

    APIF_STATUS DoQueryValHkey(void* _a, unsigned* result)
    {
        NT_QUERYVAL_HKEY_ARGS* args = (NT_QUERYVAL_HKEY_ARGS*)_a;
        //__asm int 3;
        XDBG|_S* _XOr(".apif.QueryValHkey %08x",24,228331188) % args->KeyHandle;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
        {
            __lockon__(REGISTER)
            {
                if ( !o->IsValid() )
                { *result = STATUS_INVALID_HANDLE; }
                else
                {
                    XLOG|_S* _XOr("<APIF/RGS> query value '%s' of hkey(%08x) '%s'",47,234229422)
                    %StringW(args->ValueName->Buffer,args->ValueName->Length/2)
                    %args->KeyHandle
                    %o->Path();

                    if ( o->IsProxy() )
                    {
                        HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle);
                        if ( kh )
                        {
                            args->KeyHandle = kh;
                            return APIF_CONTINUE;
                        }
                        else
                            *result = STATUS_INVALID_HANDLE;
                    }
                    else
                    {
                        StringW name(args->ValueName->Buffer,args->ValueName->Length/2);
                        name.ToUpper();
                        if ( RegValue* v = o->QueryValue(+name) )
                        {
                            *result = _QueryValue(v,args->KeyValueInformation,args->KeyValueInformationClass,args->Length,args->ResultLength);
                        }
                        else if ( !name )
                            *result = _QueryValue(+fake_valueS_,args->KeyValueInformation,args->KeyValueInformationClass,args->Length,args->ResultLength);
                        else
                            *result = STATUS_OBJECT_NAME_NOT_FOUND;
                    }
                }
            }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }

    APIF_STATUS DoEnumHkey(void* _a, unsigned* result)
    {
        NT_ENUMKEY_HKEY_ARGS* args = (NT_ENUMKEY_HKEY_ARGS*)_a;
        //__asm int 3;
        XDBG|_S* _XOr(".apif.EnumHkey %08x",20,237178595) % args->KeyHandle;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
        {
            //__asm int 3;
            if (o)
                __lockon__(REGISTER)
            {

                if ( !o->IsValid() )
                { *result = STATUS_INVALID_HANDLE; }
                else
                {
                    XLOG|_S* _XOr("<APIF/RGS> enum hkey(%08x) '%s'",32,238751259)
                    %args->KeyHandle
                    %o->Path();

                    if ( o->IsProxy() )
                    {
                        if ( REGISTER->IsVirtual() ) goto e;
                    l:
                        if ( HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle) )
                        {
                            args->KeyHandle = kh;
                            return APIF_CONTINUE;
                        }
                        else
                        e:
                            *result = STATUS_NO_MORE_ENTRIES;
                    }
                    else if ( REGISTER->IsVirtual() )
                    {
                        if ( o->SubkeysCount() > args->Index )
                        {
                            *result = _QueryInfo(o->Subkey(args->Index),
                                                 args->KeyInformation,
                                                 args->KeyInformationClass,
                                                 args->Length,
                                                 args->ResultLength);
                        }
                        else
                            *result = STATUS_NO_MORE_ENTRIES;
                    }
                    else if (args->Index >= 0 && args->Index < o->GetNormalSubkeysCount() )
                    {
                        *result = _QueryInfo(o->Subkey(args->Index),
                                             args->KeyInformation,
                                             args->KeyInformationClass,
                                             args->Length,
                                             args->ResultLength);
                    }
                    else
                    {
                        if ( args->Index == 0 && o->SubkeysCount() )
                            if ( HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle) )
                            {
                                byte_t b[256*2];
                                KEY_NODE_INFORMATION* ki = (KEY_NODE_INFORMATION*)b;
                                unsigned long kir;
                            j:
                                *result = NtEnumerateKey(kh,args->Index,KeyNodeInformation,ki,sizeof(b)-2,&kir);
                                if ( SUCCEEDED(*result) )
                                {
                                    ki->Name[ki->NameLength/2] = 0;
                                    for ( wchar_t* p = ki->Name; *p; ++p ) *p = towupper(*p);
                                    //RegEntry *ee = o->Search(ki->Name);
                                    //XLOG|_S*"%s %08x %?"%ki->Name%ee%(ee?ee->IsDeleted():false);
                                    if ( RegEntry* e = o->Search(ki->Name) )
                                    {
                                        if ( e->IsDeleted() )
                                        {
                                            ++args->Index;
                                            goto j; // virtualized subkey (may be deleted)
                                        }
                                    }
                                }
                                else
                                    return APIF_RETURN;
                            }
                        goto l;
                    }
                }
            }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }

    long _QueryInfo(RegEntry* e,void* KeyInformation, KEY_INFORMATION_CLASS KeyInformationClass, unsigned long Length,
                    unsigned long* ResultLength)
    {
        if ( !e ) return STATUS_INVALID_PARAMETER;

        struct S { long NameLength; wchar_t Name[1]; } *s;
        u32_t rl = 0;
        long result = 0;

        if ( KeyBasicInformation == KeyInformationClass )
        {
            KEY_BASIC_INFORMATION* i = (KEY_BASIC_INFORMATION*)KeyInformation;
            rl = sizeof(*i)+e->NameLength()*2;
            memset(i,0,sizeof(*i));
            s = (S*)&i->NameLength;
        }
        else if ( KeyNodeInformation == KeyInformationClass )
        {
            KEY_NODE_INFORMATION* i = (KEY_NODE_INFORMATION*)KeyInformation;
            rl = sizeof(*i)+e->NameLength()*2;
            memset(i,0,sizeof(*i));
            s = (S*)&i->NameLength;
        }
        else
        {
            result = STATUS_INVALID_PARAMETER;
        }

        if ( !result )
        {
            if ( ResultLength )
                *ResultLength = rl;
            if ( rl > Length )
                result = STATUS_BUFFER_TOO_SMALL;
            else
            {
                s->NameLength = e->NameLength()*2;
                memcpy(s->Name,e->Name(),s->NameLength);
            }
        }

        return result;
    }

    APIF_STATUS DoNotifMult(void* _a, unsigned* result)
    {
        NT_NOTYF_MULTKEY* args = (NT_NOTYF_MULTKEY*)_a;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
            __lockon__(REGISTER)
        {
            if ( o->IsProxy() )
            {
                if ( HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle) )
                {
                    args->KeyHandle = kh;
                    return APIF_CONTINUE;
                }
                else
                    goto l;
            }
            else
            {
            l:
                *result = 0;
                return APIF_RETURN;
            }
        }
        else
            return APIF_CONTINUE;
    }

    APIF_STATUS DoNotifKey(void* _a, unsigned* result)
    {
        NT_NOTYF_KEY* args = (NT_NOTYF_KEY*)_a;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
            __lockon__(REGISTER)
        {
            if ( o->IsProxy() )
            {
                if ( HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle) )
                {
                    args->KeyHandle = kh;
                    return APIF_CONTINUE;
                }
                else
                    goto l;
            }
            else
            {
            l:
                *result = 0;
                return APIF_RETURN;
            }
        }
        else
            return APIF_CONTINUE;
    }

    APIF_STATUS DoEnumValHkey(void* _a, unsigned* result)
    {
        NT_ENUMVAL_HKEY_ARGS* args = (NT_ENUMVAL_HKEY_ARGS*)_a;
        //__asm int 3;
        XDBG|_S* _XOr(".apif.EnumValHkey %08x",23,241176096) % args->KeyHandle;
        if ( HOA::R o = handles_.Get(args->KeyHandle) )
        {
            //if (o)
            __lockon__(REGISTER)
            {

                if ( !o->IsValid() )
                { *result = STATUS_INVALID_HANDLE; }
                else
                {
                    XLOG|_S* _XOr("<APIF/RGS> enum values of hkey(%08x) '%s'",42,242814553)
                    %args->KeyHandle
                    %o->Path();
                    if ( !o->IsProxy() )
                    {
                        *result = _QueryValue(+o,args->Index,
                                              args->KeyValueInformation,
                                              args->KeyValueInformationClass,
                                              args->Length,
                                              args->ResultLength);
                    }
                    else if ( REGISTER->IsVirtual() )
                    {
                        *result = STATUS_NO_MORE_ENTRIES;
                    }
                    else if ( HANDLE kh = _LookForRegKeyHandle(+o,args->KeyHandle) )
                    {
                        args->KeyHandle = kh;
                        return APIF_CONTINUE;
                        //*result = NtEnumerateValueKey(kh,
                        //              args->Index,
                        //              args->KeyValueInformationClass,
                        //              args->KeyValueInformation,
                        //              args->Length,
                        //              args->ResultLength);
                        //__asm int 3;
                    }
                    else
                    {
                        *result = STATUS_NO_MORE_ENTRIES;
                    }
                }
            }
            return APIF_RETURN;
        }
        else
            return APIF_CONTINUE;
    }

    long _QueryValue(RegEntry* e, unsigned idx, void* KeyValueInformation, KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                     unsigned long Length, unsigned long* ResultLength)
    {
        //__asm int 3;
        if ( RegValue* v = e->Value(idx) )
            return _QueryValue(v,KeyValueInformation,KeyValueInformationClass,Length,ResultLength);
        return STATUS_NO_MORE_ENTRIES;
    }

    long _QueryValue(RegValue* v, void* KeyValueInformation, KEY_VALUE_INFORMATION_CLASS KeyValueInformationClass,
                     unsigned long Length, unsigned long* ResultLength)
    {
        //__asm int 3;
        if ( KeyValueInformationClass == KeyValueBasicInformation )
        {
            long result = 0;
            *ResultLength = sizeof(KEY_VALUE_BASIC_INFORMATION) + v->Name().Length()*2;
            if ( Length < sizeof(KEY_VALUE_BASIC_INFORMATION) )
                return STATUS_BUFFER_TOO_SMALL;
            if ( Length < *ResultLength )
                result = STATUS_BUFFER_OVERFLOW;
            KEY_VALUE_BASIC_INFORMATION* ki = (KEY_VALUE_BASIC_INFORMATION*)KeyValueInformation;
            ki->TitleIndex = 0;
            ki->Type = v->Type();
            ki->NameLength = Length < *ResultLength?(Length-sizeof(KEY_VALUE_BASIC_INFORMATION)):v->Name().Length()*2;
            memcpy(ki->Name,+v->Name()+(v->Name().Length()*2-ki->NameLength)/2,ki->NameLength+2);
            return result;
        }
        else if ( KeyValueInformationClass == KeyValueFullInformation )
        {
            *ResultLength = sizeof(KEY_VALUE_FULL_INFORMATION) + v->Name().Length()*2 + v->DataLength();
            if ( Length < sizeof(KEY_VALUE_FULL_INFORMATION) )
                return STATUS_BUFFER_TOO_SMALL;
            KEY_VALUE_FULL_INFORMATION* ki = (KEY_VALUE_FULL_INFORMATION*)KeyValueInformation;
            ki->TitleIndex = 0;
            ki->Type = v->Type();
            ki->DataOffset = 0;
            ki->DataLength = v->DataLength();
            ki->NameLength = v->Name().Length()*2;
            if ( Length < sizeof(KEY_VALUE_FULL_INFORMATION) + v->Name().Length()*2 )
                return STATUS_BUFFER_OVERFLOW;
            memcpy(ki->Name,+v->Name(),ki->NameLength);
            if ( Length < *ResultLength )
                return STATUS_BUFFER_OVERFLOW;
            ki->DataOffset = sizeof(KEY_VALUE_FULL_INFORMATION) + v->Name().Length()*2;
            memcpy((byte_t*)ki + ki->DataOffset,v->Data(),ki->DataLength);
            return 0;
        }
        else if ( KeyValueInformationClass == KeyValuePartialInformation )
        {
            *ResultLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION) + v->DataLength() - 1;
            if ( Length < sizeof(KEY_VALUE_PARTIAL_INFORMATION) )
                return STATUS_BUFFER_TOO_SMALL;
            KEY_VALUE_PARTIAL_INFORMATION* ki = (KEY_VALUE_PARTIAL_INFORMATION*)KeyValueInformation;
            ki->TitleIndex = 0;
            ki->Type = v->Type();
            ki->DataLength = v->DataLength();
            unsigned l = Length < *ResultLength ? *ResultLength - Length: 0;
            memcpy(ki->Data,v->Data(),ki->DataLength-l);
            if ( Length < *ResultLength )
                return STATUS_BUFFER_OVERFLOW;
            return 0;
        }
        else
            __asm int 3;
        return STATUS_NO_MORE_ENTRIES;
    }

    struct REGKEY_HANDLE_RECORD { HANDLE eh; RegEntry* e; u32_t stamp; };
    enum { REGKEY_HANDLE_COUNT = 16 };
    REGKEY_HANDLE_RECORD rkh_recs_[REGKEY_HANDLE_COUNT];// = {0,};

    HANDLE _LookForRegKeyHandle(RegEntry* e,HANDLE /*eh*/ = 0,unsigned /*rw*/ = 0)
    {
        static int stamp = 0;
        if ( !e ) return 0;
        //if ( rw > 0 ) rw = 1;
        REGKEY_HANDLE_RECORD* minrkh = rkh_recs_;
        REGKEY_HANDLE_RECORD* rkh = 0;

        //if ( REGISTER->IsVirtual() ) return 0;

        for ( int i = 0; i < REGKEY_HANDLE_COUNT; ++i )
        {
            if ( rkh_recs_[i].e == e )
            { rkh_recs_[i].stamp = ++stamp; return rkh_recs_[i].eh; }
            else
            { if ( minrkh->stamp > rkh_recs_[i].stamp ) minrkh =  rkh_recs_+i; }
        }

        if ( minrkh->eh )
            NtClose(minrkh->eh);

        minrkh->stamp = 0;
        minrkh->eh = 0;
        minrkh->e = 0;

        StringW rpath; e->Path(rpath);

        OBJECT_ATTRIBUTES oa;
        UNICODE_STRING us = { rpath.Length()*2, rpath.Length()*2+2, (wchar_t*)+rpath };
        InitializeObjectAttributes(&oa,&us,OBJ_CASE_INSENSITIVE,0,0);

        if ( long st = NtOpenKey(&minrkh->eh,MAXIMUM_ALLOWED,&oa) )
            //if ( long st = NtOpenKey(&minrkh->eh,KEY_READ|(rw?KEY_WRITE:0),&oa) )
        {
            if ( st != STATUS_OBJECT_NAME_NOT_FOUND )
            {
                XLOG|_S* _XOr("<ERR> failed to open key '%s' with status %08x ['apif_register.cpp':%d]",72,245894760) 
			%rpath %st %__LINE__;
                //__asm int 3;
            }
            minrkh->eh = 0;
        }
        else
        {
            XDBG|_S* _XOr("<APIF/RGS> open shadow key %08x '%s'",37,251556310) %minrkh->eh %rpath;
            minrkh->stamp = ++stamp;
            minrkh->e = e;
        }
        return minrkh->eh;
    }

    void _ClearRegKeyHandle(RegEntry* e)
    {
        for ( int i = 0; i < REGKEY_HANDLE_COUNT; ++i )
            if ( rkh_recs_[i].e == e )
            {
                if ( rkh_recs_[i].eh )
                    NtClose(rkh_recs_[i].eh);
                memset(&rkh_recs_[i],0,sizeof(rkh_recs_[i]));
                return;
            }
    }

    virtual APIF_STATUS DoQuerySecurity(void* _a,unsigned* result)
    {
        NT_QUESEQ_ARGS* args = (NT_QUESEQ_ARGS*)_a;
        XDBG|_S* _XOr(".apif.QueryKeySecurity %08x",28,254243327) % args->ObjectHandle;
        if ( HOA::R o = handles_.Get(args->ObjectHandle) )
            __lockon__(REGISTER)
        {
            if ( !o->IsValid() )
            { *result = STATUS_INVALID_HANDLE; }
            else
            {
                XLOG|_S* _XOr("<APIF/RGS> query security info of hkey(%08x) '%s'",50,251687168)
                %args->ObjectHandle
                %o->Path();
                if ( o->IsProxy() )
                {
                    if ( HANDLE kh = _LookForRegKeyHandle(+o,args->ObjectHandle) )
                        args->ObjectHandle = kh;
                }
                else if ( HANDLE kh = _LookForRegKeyHandle(REGISTER->USER) )
                    args->ObjectHandle = kh;
            }
        }
        return APIF_CONTINUE;
    }

    virtual APIF_STATUS DoCall(int apif_id, void* args, unsigned* result)
    {
        if ( !REGISTER->IsEnabled() ) switch (apif_id)
            {
                case APIF_CREATEHKEY:
                    return DoCreateHkey_1(args,result);
                case APIF_OPENHKEY:
                case APIF_OPENHKEYEX:
                    return DoOpenHkey_1(args,result);
            }
        else switch (apif_id)
            {
                case APIF_CREATEHKEY:
                    return DoCreateHkey(args,result);
                case APIF_OPENHKEY:
                case APIF_OPENHKEYEX:
                    return DoOpenHkey(args,result);
                case APIF_CLOSE:
                    return DoCloseHkey(args,result);
                case APIF_SETVALHKEY:
                    return DoSetValHkey(args,result);
                case APIF_QUEKEY:
                    return DoQueryHkey(args,result);
                case APIF_QUEVALKEY:
                    return DoQueryValHkey(args,result);
                case APIF_ENUKEY:
                    return DoEnumHkey(args,result);
                case APIF_ENUVALKEY:
                    return DoEnumValHkey(args,result);
                case APIF_NOTIFMULTKEY:
                    return DoNotifMult(args,result);
                case APIF_NOTIFKEY:
                    return DoNotifKey(args,result);
                case APIF_DELKEY:
                    return DoDeleteHkey(args,result);
                case APIF_DELKEYVAL:
                    return DoDeleteHkeyVal(args,result);
                case APIF_DUPLICATE:
                    return DoDuplicateKey(args,result);
                case APIF_QUESEQ:
                    return DoQuerySecurity(args,result);
            }
        return APIF_CONTINUE;
    }

    RegObjectFilter(RegObjectFilter const&);

    virtual void LogState()
    {
        __lockon__(REGISTER)
        {
            HOA::R r;
            HANDLE h;
            int i = 0;
            XLOG|_S* _XOr("<APIF/RGS> filtered: %d, virtual %d ",37,259813716)
            %REGISTER->IsFiltered() %REGISTER->IsVirtual();
            XLOG|_XOr("<APIF/RGS> --- begin of registry state --- ",44,262566270);
            while ( handles_.NextRecord(&i,&h,&r) )
                XLOG|_S* _XOr("... [%08x] %5s %s",18,261386380)
                %h
                %(r->IsProxy()?_XOr("proxy",6,266367136):_XOr("node",5,266956969))
                %r->Path();
            XLOG|_XOr("<APIF/RGS> --- persistent nodes --- ",37,267546834);
            for ( int i = 0; i < REGISTER->Stored().Count() ; ++i )
                XLOG|_S* _XOr("... %s",7,265908473) %REGISTER->Stored()[i]->Path();
            XLOG|_XOr("<APIF/RGS> --- end of registry state --- ",42,271413477);
        }
    }

};

RegObjectFilter apif_robjfilt = RegObjectFilter();

void APIF_RegisterRegObjectHooks(bool filtered,bool logrgloaging)
{
    APIF->Push(&apif_robjfilt);
    REGISTER->SetFilteredMode(filtered);
    //REGISTER->LogRgLoading(logrgloaging);
};

long __stdcall APIF_RegisterWriteToFile(unsigned flags,wchar_t const* filename,wchar_t const* filter)
{
    __lockon__(REGISTER)
    if ( DataStreamPtr ds = DataSource->Open(filename,STREAM_CREATE|STREAM_WRITE) )
    {
        REGISTER->Write(ds,filter,flags);
    }
    return 0;
}

BufferT<byte_t> g_serelized_register;

long __stdcall APIF_RegisterSerilize(unsigned flags,wchar_t const* filter,char** p,int* len)
{
    __lockon__(REGISTER)
    {
        ArrayDataStream ads;
        g_serelized_register.Resize(0);
        g_serelized_register.Reserve(16*1024);
        ads.SwapBuffer(g_serelized_register);
        REGISTER->Write(RccRefe((DataStream*)&ads),filter,flags|TEGGOVBOX_ONELINE_FORMAT);
        ads.UtfPrint(L"\n");
        ads.SwapBuffer(g_serelized_register);
        *p = (char*)+g_serelized_register;
        *len = g_serelized_register.Count();
    }
    return 0;
}

void REGISTER_LoadRegData(byte_t* data,unsigned count)
{
    if ( count )
        __lockon__(REGISTER)
    {
        MemDataStream mds(data,count,STREAM_READ);
        REGISTER->Load(RccRefe((DataStream*)&mds));
        //if (iflog) apif_robjfilt.LogState();
    }
}


int Get_Registry_Virtualization_Mode()
{
    int mode = 0;
    __lockon__(REGISTER)
    {
        if ( !REGISTER->IsEnabled() )
            mode = REGISTRY_DISABLE_VIRTUALIZATION;
        else if ( REGISTER->IsFiltered() )
            mode = REGISTRY_PARTIAL_VIRTUALIZATION;
        else if ( REGISTER->IsVirtual() )
            mode = REGISTRY_VIRTUAL_VIRTUALIZATION;
        else
            mode = REGISTRY_FULL_VIRTUALIZATION;
    }
    return mode;
}

int Set_Registry_Virtualization_Mode(int mode)
{
    int old = Get_Registry_Virtualization_Mode();
    xlog|_S*"change rv mode %d ->%d"%old%mode;
    __lockon__(REGISTER)
    {
        if ( mode == REGISTRY_DISABLE_VIRTUALIZATION )
        {
            REGISTER->Disable();
        }
        else if ( mode == REGISTRY_PARTIAL_VIRTUALIZATION )
        {
            REGISTER->Enable();
            REGISTER->SetFilteredMode(true);
            REGISTER->SetVirtualMode(false);
        }
        else if ( mode == REGISTRY_FULL_VIRTUALIZATION )
        {
            REGISTER->Enable();
            REGISTER->SetFilteredMode(false);
            REGISTER->SetVirtualMode(false);
        }
        else if ( mode == REGISTRY_VIRTUAL_VIRTUALIZATION )
        {
            REGISTER->Enable();
            REGISTER->SetFilteredMode(false);
            REGISTER->SetVirtualMode(true);
        }
        apif_robjfilt.LogState();
    }
    return old;
}
