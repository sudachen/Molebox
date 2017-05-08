
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#if !defined __506DA0C7_943D_4E7C_A382_00B907BB5CD8__
#define __506DA0C7_943D_4E7C_A382_00B907BB5CD8__

#if !defined CXX_NO_MSCOM_SUPPORT

#include "_specific.h"
#include "hinstance.h"
#include "string.h"
#include "com_generic.h"

#define ComRegIfSok(hr,expr) FAILED(hr) ? hr : (expr)

namespace teggo
{

    _TEGGO_EXPORTABLE HRESULT CXX_STDCALL ComRegWriteStr(pchar_t root,pchar_t key,pchar_t subkey,pchar_t value,
            HKEY ROOT=HKEY_CLASSES_ROOT);
    _TEGGO_EXPORTABLE HRESULT CXX_STDCALL ComRegDeleteKey(pchar_t key,HKEY ROOT=HKEY_CLASSES_ROOT);
    _TEGGO_EXPORTABLE HRESULT CXX_STDCALL ComRegLoadTypeLib(ITypeLib** pI,pwide_t p = 0,HinstanceType& hinstance = Hinstance);
    _TEGGO_EXPORTABLE HRESULT CXX_STDCALL ComRegRegisterTypeLib(HinstanceType& hinstance = Hinstance);
    _TEGGO_EXPORTABLE HRESULT CXX_STDCALL ComRegUnregisterTypeLib(HinstanceType& hinstance = Hinstance);
    _TEGGO_EXPORTABLE HRESULT CXX_STDCALL ComRegRegisterCoClass(const GUID& clsid,pchar_t progID,pchar_t desc,long ver,
            HinstanceType& hinstance = Hinstance);
    _TEGGO_EXPORTABLE HRESULT CXX_STDCALL ComRegUnregisterCoClass(const GUID& clsid,pchar_t progID,long ver);

    _TEGGO_EXPORTABLE const wchar_t* CXX_STDCALL GUIDToRegString(const GUID& guid,BaseStringT<wchar_t>& s);
    _TEGGO_EXPORTABLE const char* CXX_STDCALL GUIDToRegString(const GUID& guid,BaseStringT<char>& s);

    typedef char _GUID_char48_t[48];
    _TEGGO_EXPORTABLE pchar_t CXX_STDCALL GUIDToRegStringA(const GUID& guid,
            mutable_t<_GUID_char48_t> const& val = mutable_t<_GUID_char48_t>());
    typedef wchar_t _GUID_wchar48_t[48];
    _TEGGO_EXPORTABLE pwide_t CXX_STDCALL GUIDToRegStringW(const GUID& guid,
            mutable_t<_GUID_wchar48_t> const& val = mutable_t<_GUID_wchar48_t>());
}

#endif // !defined CXX_NO_MSCOM_SUPPORT

#endif // __506DA0C7_943D_4E7C_A382_00B907BB5CD8__
