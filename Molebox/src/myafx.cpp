
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "myafx.h"

int _MAJOR_VERSION = 0;
int _MINOR_VERSION = 0;
int _BUILD_NUMBER  = 0;

pwide_t Translate(int i,pwide_t text) 
  { 
    return text;
  }

#include "tr.inl"

pwide_t MessageValue(int i )
  {
    static FlatmapT<int,StringW> *xmap = 0;
    if ( !xmap )
      {
        xmap = new FlatmapT<int,StringW>();
        for ( int i = 0; i < _TR_COUNT_; ++i)
          {
            StringW line = _TR_[i];
            StringW n = line.TillAndTrim(L':',true); n.Trim();
            int lN = ToLong(+n,0);
            line.Trim();
            line.ReplaceS(L"\\n",L"\n");
            xmap->Put(lN,line);
          }
      }
    if ( xmap )
      if ( StringW *val = xmap->Get(i) )
        return +*val;
    return L"~~~notfound~~~~";
  }
  
void DirList_(StringParam rfold, StringW const &prefix, StringParam patt, ArrayT<StringW> &names, u32_t flgs)
  {
    WIN32_FIND_DATAW wdta;
    StringW searchS = JoinPath(rfold,prefix+patt);
    HANDLE fh = FindFirstFileW(+searchS,&wdta);
    if ( fh != INVALID_HANDLE_VALUE ) 
      {
        do
          {
            if ( wdta.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
              {
                if ( wdta.cFileName[0] == '.' 
                  && ( wdta.cFileName[1] == '\0' || ( wdta.cFileName[1] == '.' && wdta.cFileName[2] == '\0' )) )
                  continue;
                if ( !(flgs&DIRLIST_FILESONLY) )
                  names.Push(prefix + wdta.cFileName);
                if ( flgs&DIRLIST_RECURSIVE )
                  DirList_(rfold,prefix+wdta.cFileName+"/",patt,names,flgs);
              }
            else
              if ( !(flgs&DIRLIST_DIRESONLY) )
                names.Push(prefix + wdta.cFileName);
          }
        while ( FindNextFileW(fh,&wdta) );
        FindClose(fh);
      }
  }
  
void DirList(StringParam rfold, StringParam patt, ArrayT<StringW> &names, u32_t flgs)
  {
    DirList_(rfold, StringW(""), patt, names, flgs);
  }
