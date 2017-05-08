
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#ifndef ___d650f735_c47d_4c3a_8135_24450a16035d___
#define ___d650f735_c47d_4c3a_8135_24450a16035d___

#include "_specific.h"
#include "string.h"
#include "hinstance.h"
#include "sysutil.h"

namespace teggo
{

    template <class Tchr>
    CXX_NO_INLINE int GetIdxOfPathSplitR(Tchr const* path)
    {
        int i0 = StrFindRChar(path,Tchr('\\'),-1,-1);
        int i1 = StrFindRChar(path,Tchr('/'),-1,-1);
        if ( i0 < i1 ) i0 = i1;
        //while ( i0 >= 1 && (path[i0-1] == '\\' || path[i0-1] == '/') ) --i0;
        //wprintf(L"-%s\n",+StringW(path,i0));
        return i0;
    }

    template <class Tchr>
    CXX_NO_INLINE int GetIdxOfPathSplitL(Tchr const* path)
    {
        int i0 = StrFindChar(path,Tchr('\\'),0,-1);
        int i1 = StrFindChar(path,Tchr('/'),0,-1);
        if ( i0 < 0 || ( i0 > i1 && i1 >= 0 ) ) i0 = i1;
        return i0;
    }

    template <class Tchr>
    CXX_NO_INLINE StringT<Tchr> GetBasenameOfPath(Tchr const* path)
    {
        int i = GetIdxOfPathSplitR(path);
        if ( i >= 0 )
            path += (i+1);
        return StringT<Tchr>(path);
    }

    inline StringT<char> GetBasenameOfPath(BaseStringT<char> const& path)
    {
        return GetBasenameOfPath(+path);
    }

    inline StringW GetBasenameOfPath(StringParam path)
    {
        return GetBasenameOfPath(+path);
    };

    template <class Tchr>
    CXX_NO_INLINE StringT<Tchr> GetDirectoryOfPath(Tchr const* path)
    {
        int i = GetIdxOfPathSplitR(path);
        if ( i > 0 )
        {
            //while ( i > 0 && ( path[i] == '\\' || path[i] == '/' ) ) --i;
            return StringT<Tchr>(path,i);
        }
        else
            return StringT<Tchr>();
    }

    inline StringT<char> GetDirectoryOfPath(BaseStringT<char> const& path)
    {
        return GetDirectoryOfPath(+path);
    }

    inline StringW GetDirectoryOfPath(StringParam path)
    {
        return GetDirectoryOfPath(+path);
    };

    template <class Tchr>
    CXX_NO_INLINE ArrayT<StringT<Tchr>> SplitPath(Tchr const* path)
    {
        ArrayT<StringT<Tchr>> a;

        for (int i = GetIdxOfPathSplitL(path); i >= 0; i = GetIdxOfPathSplitL(path) )
        {
            if (i>0) // skiping empty parts
            {
                a.Append(StringT<Tchr>(path,i));
            }
            path+=(i+1);
        }

        if ( StrLen(path) )
            a.Append(path);

        return ArrayT<StringT<Tchr>> (a, SWAP_CONTENT);
    }

    inline ArrayT<StringT<char>> SplitPath(BaseStringT<char> const& path)
    {
        return SplitPath(+path);
    }

    inline ArrayT<StringW> SplitPath(StringParam path)
    {
        return SplitPath(+path);
    }

    enum { PATH_SEPARATOR = '\\' };

    CXX_FAKE_INLINE StringW JoinPath(StringParam dir,StringParam fname)
    {
        if ( !+dir || !*+dir ) return StringW(+fname);
        else
        {
            StringW S = dir;
            if ( !S.EndsWith(L"/") && !S.EndsWith(L"\\") )
                S.Append(wchar_t(PATH_SEPARATOR));
            return S.Append(+fname);
        }
    }

    template <class Tchr>
    CXX_NO_INLINE bool IsFullPath(Tchr const* p)
    {
        return ( *p != '\0' && p[1] == ':' && (p[2] == '\\' || p[2] == '/') )
               || (  ( p[0] == '\\' || p[0] == '/' )
                     && ( p[1] == '\\' || p[1] == '/' || p[1] == '?' )
                     &&   p[2] == '?' )
               ;
    }

    inline bool IsFullPath(StringParam path)
    {
        return IsFullPath(+path);
    }

    template <class Tchr>
    CXX_NO_INLINE StringT<Tchr> ResolveFullPath(Tchr const* base, Tchr const* p)
    {
        if ( IsFullPath(p) ) return StringT<Tchr>(p);
        else
            return JoinPath(base,p);
    }

    inline StringW ResolveFullPath(StringParam base, StringParam path)
    {
        return ResolveFullPath(+base,+path);
    }

    CXX_FAKE_INLINE StringW AtAppFolder(StringParam path)
    {
        if ( IsFullPath(path) )
            return +path;
        return JoinPath(Hinstance->GetMainModuleDir(),path);
    }

    CXX_FAKE_INLINE StringW AtCurrentDir(StringParam path)
    {
        if ( IsFullPath(path) )
            return +path;
        return JoinPath(SysGetCurrentDirectory(),path);
    }

    CXX_FAKE_INLINE StringW AtSameFolder(StringParam at,StringParam path)
    {
        if ( IsFullPath(path) )
            return +path;
        StringW fold = GetDirectoryOfPath(at);
        return JoinPath(fold,path);
    }

    CXX_FAKE_INLINE StringW AtLocalUserAppFolder(StringParam path)
    {
        if ( IsFullPath(path) )
            return +path;
        StringW fold = SysGetLocalUserAppFolder();
        return JoinPath(fold,path);
    }

    CXX_FAKE_INLINE StringW AtUserAppFolder(StringParam path)
    {
        if ( IsFullPath(path) )
            return +path;
        StringW fold = SysGetUserAppFolder();
        return JoinPath(fold,path);
    }

    CXX_FAKE_INLINE bool CreateDirectoriesIfNeed(StringParam filename, bool treatsAsDirname=false)
    {
        StringW dirpath = treatsAsDirname ? StringW(filename) : GetDirectoryOfPath(filename);
        if ( dirpath && !(dirpath.Length() == 2 && dirpath[1] == ':') )
        {
            //wprintf(L"%s\n",+dirpath);
            if ( CreateDirectoriesIfNeed(dirpath) )
                if ( !FileExists(dirpath) )
                    return CreateDirectoryW(+dirpath,0);
        }
        return true;
    }

    CXX_FAKE_INLINE bool UnlinkFile(StringParam filename)
    {
        return DeleteFileW(+filename);
    }

} //namespace

#endif /*___d650f735_c47d_4c3a_8135_24450a16035d___*/
