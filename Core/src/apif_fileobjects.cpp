
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

struct NT_DUPLICATE_ARGS
  {
    HANDLE                SourceProcessHandle;
    HANDLE                SourceHandle;
    HANDLE                TargetProcessHandle;
    HANDLE                *TargetHandle;
    ACCESS_MASK           DesiredAccess;
    BOOLEAN               InheritHandle;
    ULONG                 Options;
  };

struct NT_CREATE_FILE_ARGS
  {
    HANDLE               *FileHandle;
    ACCESS_MASK          DesiredAccess;
    OBJECT_ATTRIBUTES    *ObjectAttributes;
    IO_STATUS_BLOCK      *IoStatusBlock;
    LARGE_INTEGER        *AllocationSize /*OPTIONAL*/;
    ULONG                FileAttributes;
    ULONG                ShareAccess;
    ULONG                CreateDisposition;
    ULONG                CreateOptions;
    VOID                 *EaBuffer /*OPTIONAL*/;
    ULONG                EaLength;
  };

struct NT_OPEN_FILE_ARGS
  {
    HANDLE               *FileHandle;
    ACCESS_MASK          DesiredAccess;
    OBJECT_ATTRIBUTES    *ObjectAttributes;
    IO_STATUS_BLOCK      *IoStatusBlock;
    ULONG                ShareAccess;
    ULONG                OpenOptions;
  };


struct NT_WRITE_FILE_ARGS
  {
    HANDLE               FileHandle;
    HANDLE               Event /*OPTIONAL*/;
    PIO_APC_ROUTINE      ApcRoutine /*OPTIONAL*/;
    VOID                 *ApcContext /*OPTIONAL*/;
    IO_STATUS_BLOCK      *IoStatusBlock;
    VOID                 *Buffer;
    ULONG                Length;
    LARGE_INTEGER        *ByteOffset /*OPTIONAL*/;
    ULONG                *Key /*OPTIONAL*/;
  };

struct NT_READ_FILE_ARGS
  {
    HANDLE               FileHandle;
    HANDLE               Event /*OPTIONAL*/;
    PIO_APC_ROUTINE      ApcRoutine /*OPTIONAL*/;
    VOID                 *ApcContext /*OPTIONAL*/;
    IO_STATUS_BLOCK      *IoStatusBlock;
    VOID                 *Buffer;
    ULONG                Length;
    LARGE_INTEGER        *ByteOffset /*OPTIONAL*/;
    ULONG                *Key /*OPTIONAL*/;
  };

struct NT_QUE_FILINFO_ARGS
  {
    HANDLE               FileHandle;
    IO_STATUS_BLOCK      *IoStatusBlock;
    VOID                 *FileInformation;
    ULONG                Length;
    XFILE_INFORMATION_CLASS FileInformationClass;
  };

struct NT_SET_FILINFO_ARGS
  {
    HANDLE               FileHandle;
    IO_STATUS_BLOCK      *IoStatusBlock;
    VOID                 *FileInformation;
    ULONG                Length;
    XFILE_INFORMATION_CLASS FileInformationClass;
  };

struct NT_CLOSE_ARGS
  {
    HANDLE               FileHandle;
  };

struct NT_QUE_VOLINFO_ARGS
  {
    HANDLE               FileHandle;
    IO_STATUS_BLOCK      *IoStatusBlock;
    VOID                 *FileSystemInformation;
    ULONG                Length;
    FS_INFORMATION_CLASS FileSystemInformationClass;
  };

struct NT_QUE_DIRFILE_ARGS
  {
    HANDLE               FileHandle;
    HANDLE               Event;
    PIO_APC_ROUTINE      ApcRoutine;
    VOID                 *ApcContext;
    IO_STATUS_BLOCK      *IoStatusBlock;
    VOID                 *FileInformation;
    ULONG                Length;
    FILE_INFORMATION_CLASS FileInformationClass;
    BOOLEAN              ReturnSingleEntry;
    UNICODE_STRING       *FileMask;
    BOOLEAN              RestartScan;
  };

struct NT_QUE_FILEATTR_ARGS
  {
    OBJECT_ATTRIBUTES    *ObjectAttributes;
    FILE_BASIC_INFORMATION *FileAttributes;
  };

struct NT_LOCKFILE_ARGS
  {
    HANDLE               FileHandle;
    HANDLE               *LockGrantedEvent;
    PIO_APC_ROUTINE      ApcRoutine;
    VOID                 *ApcContext;
    IO_STATUS_BLOCK      *IoStatusBlock;
    LARGE_INTEGER        *ByteOffsetL;
    LARGE_INTEGER        *Length;
    ULONG                *Key;
    BOOLEAN              ReturnImmediately;
    BOOLEAN              ExclusiveLock;
  };

struct NT_UNLOCKFILE_ARGS
  {
    HANDLE               FileHandle;
    IO_STATUS_BLOCK      *IoStatusBlock;
    LARGE_INTEGER        *ByteOffsetL;
    LARGE_INTEGER        *Length;
    ULONG                *Key;
  };

struct NT_NOTIFCHGF_ARGS
  {
    HANDLE               FileHandle;
    HANDLE               Event;
    PIO_APC_ROUTINE      ApcRoutine;
    VOID                 *ApcContext;
    IO_STATUS_BLOCK      *IoStatusBlock;
    VOID                 *Buffer;
    ULONG                BufferSize;
    ULONG                CompletionFilter;
    BOOLEAN              WatchTree;
  };

bool fsFilterIsEnabled = true;
unsigned __stdcall CoreAPI_EnableFsFilter(unsigned enable)
  {
    unsigned r = fsFilterIsEnabled ? 1 : 0;
    fsFilterIsEnabled = !!enable;
    return r;
  };

extern bool exeFilterIsEnabled;
unsigned __stdcall CoreAPI_EnableExeFilter(unsigned enable)
  {
    unsigned r = exeFilterIsEnabled ? 1 : 0;
    exeFilterIsEnabled = !!enable;
    return r;
  };

unsigned __stdcall CoreAPI_Mount(pwide_t pakagemask, pchar_t passwd, unsigned atAppfold)
  {
    return VFS->Mount(pakagemask,passwd,atAppfold);
  }

unsigned __stdcall CoreAPI_Unmount(pwide_t pakagemask)
  {
    return 0;
  }

inline void Delete(FioProxy *p) { p->Release(); }
typedef HandledObjectsArray<FioProxy,u32_t> HOA;
typedef FlatmapT< HANDLE, VfsDirlistPtr > SUBJECT;

struct FileObjectFilter : APIfilter
  {

    HOA handles_;
    Tlocker l_;
    SUBJECT subject_;

    FileObjectFilter() {};

    struct OA : OBJECT_ATTRIBUTES
      {
        OA(OBJECT_ATTRIBUTES *oa) 
          {
            memset(reserve,0,sizeof(reserve));
            memcpy(this,oa,oa->Length);
            uS = *oa->ObjectName;
            ObjectName = &uS;
            uS.MaximumLength = uS.Length + 2;
          }
        byte_t reserve[8];
        UNICODE_STRING uS;
        StringW strbuf;
      };

    static void CopyRange(StringW &b, wchar_t *S, int L)
      {
        b.Resize(L);
        memcpy((wchar_t*)+b,S,L*2);
      }

    static void AppendCopyRange(StringW &b, wchar_t *S, int L)
      {
        int oL = b.Length();
        b.Resize(oL+L);
        memcpy((wchar_t*)+b+oL,S,L*2);
      }

    FioProxyPtr NormalizeOAnGetProxy(OA *oa,StringW &internal,bool *inBox)
      {
        //for ( int i = 2; i < oa->ObjectName->Length/2; ++i )
        //  if ( oa->ObjectName->Buffer[i] == '\\' )
        //    {
        //      has_dir = true;
        //      break;
        //    }
        
        StringW &strbuf = oa->strbuf;
        
        if ( oa->RootDirectory )
          if ( HOA::R o = handles_.Get(oa->RootDirectory) )
            {
              *inBox = true;
              if ( !o->IsRoot() )
                {
                  strbuf = o->ComposePath1();
                  AppendCopyRange(strbuf,oa->ObjectName->Buffer,oa->ObjectName->Length/2);
                  oa->ObjectName->Buffer = (wchar_t*)+strbuf;
                }
              else
                {
                  CopyRange(strbuf,oa->ObjectName->Buffer,oa->ObjectName->Length/2);
                  oa->ObjectName->Buffer = (wchar_t*)+strbuf;
                }
              oa->ObjectName->MaximumLength = ( oa->ObjectName->Length = strbuf.Length()*2 ) + 2;
              oa->RootDirectory = VFS->AppFolderHandle();
              return VFS->GetProxy(oa->ObjectName->Buffer,&internal,true);
            }
          else
            {
              CopyRange(strbuf,oa->ObjectName->Buffer,oa->ObjectName->Length/2);
              oa->ObjectName->Buffer = (wchar_t*)+strbuf;
              oa->ObjectName->MaximumLength = ( oa->ObjectName->Length = strbuf.Length()*2 ) + 2;

              if ( oa->RootDirectory == _CURDIRHANDLE() )
                {
                  FioProxyPtr prox = VFS->GetProxy(oa->ObjectName->Buffer,&internal,false);
                  *inBox = !!prox;
                  return prox;
                }
              else
                {
                  *inBox = false;
                  //__asm int 3;
                  XLOG|_S*_XOr("oppss, unknown path to %08x/%s",31,67898372)
                    %oa->RootDirectory %oa->ObjectName->Buffer;
                  return FioProxyPtr(0);
                }
            }
        else
          {
            CopyRange(strbuf,oa->ObjectName->Buffer,oa->ObjectName->Length/2);
            oa->ObjectName->Buffer = (wchar_t*)+strbuf;

            if ( inBox )
              {
                StringW internal_path;
                StringW dirname = GetDirectoryOfPath(oa->ObjectName->Buffer);
                *inBox = dirname && !!VFS->GetProxy(+dirname,&internal_path,false);
              }
            oa->ObjectName->MaximumLength = ( oa->ObjectName->Length = strbuf.Length()*2 ) + 2;
            return VFS->GetProxy(oa->ObjectName->Buffer,&internal,false);
          }
      }

    int Conflict(FioProxyPtr &p, OBJECT_ATTRIBUTES *oa)
      {
        if ( int conflict = p->Conflict() )
          {
            FILE_BASIC_INFORMATION fi;
            if ( SUCCEEDED(NtQueryAttributesFile(oa,&fi)) )
              return conflict;
          }
        return 0;
      }

    APIF_STATUS DoCreateFile(void *_a, unsigned *result)
      {
        NT_CREATE_FILE_ARGS *args = (NT_CREATE_FILE_ARGS*)_a;

        XDBG|_S*_XOr(".apif.CreateFile '%s'",22,69274659) 
          %StringW(args->ObjectAttributes->ObjectName->Buffer,args->ObjectAttributes->ObjectName->Length/2);
        
        bool inBox = false;
        StringW internal_path;
        OA oa = args->ObjectAttributes;
        
        __lockon__(VFS)
          if ( FioProxyPtr p = NormalizeOAnGetProxy(&oa,internal_path,&inBox) )
            {
              
              switch ( Conflict(p,&oa) )
                { 
                  case 1:
                    goto native;
                  case 2:
                    *result = STATUS_ACCESS_DENIED;
                    return APIF_RETURN;
                }

              if ( args->CreateDisposition != FILE_OPEN ) 
                {
                  *result = STATUS_OBJECT_NAME_COLLISION;
                }
              else
                {
                  *args->FileHandle = handles_.Put(Refe(+p));
                  *result = 0;
                  XLOG|_S*_XOr("<APIF/VFS> open('%s\\%s') -> %08x",33,72092740)
                          %p->PackageName() %internal_path %*args->FileHandle;
                }
              return APIF_RETURN;
            }
          else
            {
          native:
              *result = NtCreateFile(args->FileHandle,
                                    args->DesiredAccess,
                                    //args->ObjectAttributes,
                                    &oa,
                                    args->IoStatusBlock,
                                    args->AllocationSize,
                                    args->FileAttributes,
                                    args->ShareAccess,
                                    args->CreateDisposition,
                                    args->CreateOptions,
                                    args->EaBuffer,
                                    args->EaLength);
              
              if ( *result == STATUS_OBJECT_PATH_NOT_FOUND && inBox )
                *result = STATUS_OBJECT_NAME_NOT_FOUND;
              
              return APIF_RETURN;
            }
      }

    APIF_STATUS DoQueFileAttr(void *_a, unsigned *result)
      {
        NT_QUE_FILEATTR_ARGS *args = (NT_QUE_FILEATTR_ARGS*)_a;

        XDBG|_S*_XOr(".apif.QueAttrFile '%s'",23,73796706)
          %StringW(args->ObjectAttributes->ObjectName->Buffer,args->ObjectAttributes->ObjectName->Length/2);
        
        bool inBox = false;
        StringW internal_path;
        OA oa = args->ObjectAttributes;

        __lockon__(VFS)
          if ( FioProxyPtr p = NormalizeOAnGetProxy(&oa,internal_path,&inBox) )
            {
              switch ( Conflict(p,&oa) )
                { 
                  case 1: 
                    goto native;
                  case 2:
                    *result = STATUS_ACCESS_DENIED;
                    return APIF_RETURN;
                }
              
              FILE_BASIC_INFORMATION *i = (FILE_BASIC_INFORMATION *)args->FileAttributes;
              i->FileAttributes = p->IsDirectory()?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_READONLY;
              *result = 0;
              XLOG|_S*_XOr("<APIF/VFS> query attr of '%s\\%s' -> %08x",41,76484485)
                      %p->PackageName() %internal_path %i->FileAttributes;
              return APIF_RETURN;
            }
          else
            {
          native:
              if ( SUCCEEDED(*result = NtQueryAttributesFile(&oa,args->FileAttributes)) )
                ; // nothing
              else if ( *result == STATUS_OBJECT_PATH_NOT_FOUND && inBox )
                *result = STATUS_OBJECT_NAME_NOT_FOUND;
              return APIF_RETURN;
            }
      }

    APIF_STATUS DoQueFileAttrFull(void *_a, unsigned *result)
      {
        NT_QUE_FILEATTR_ARGS *args = (NT_QUE_FILEATTR_ARGS*)_a;

        XDBG|_S*_XOr(".apif.QueAttrFileFull '%s'",27,79564762) 
          %StringW(args->ObjectAttributes->ObjectName->Buffer,args->ObjectAttributes->ObjectName->Length/2);

        bool inBox = false;
        StringW internal_path;
        OA oa = args->ObjectAttributes;

        __lockon__(VFS)
          if ( FioProxyPtr o = NormalizeOAnGetProxy(&oa,internal_path,&inBox) )
            {
              switch ( Conflict(o,&oa) )
                { 
                  case 1: 
                    goto native;
                  case 2:
                    *result = STATUS_ACCESS_DENIED;
                    return APIF_RETURN;
                }
              *result = 0;
              FILE_NETWORK_OPEN_INFORMATION *i = (FILE_NETWORK_OPEN_INFORMATION*)args->FileAttributes;
              memset(i,0,sizeof(FILE_NETWORK_OPEN_INFORMATION));
              i->FileAttributes = o->IsDirectory()?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_READONLY;
              i->LastWriteTime = o->GetMtime();
              i->CreationTime = o->GetCtime();
              i->LastAccessTime = i->LastWriteTime;
              i->ChangeTime = i->LastWriteTime;
              i->AllocationSize.QuadPart = i->EndOfFile.QuadPart = o->Size();
              XLOG|_S*_XOr("<APIF/VFS> query full attr of '%s\\%s' -> %08x",46,81661946)
                      %o->PackageName() %internal_path %i->FileAttributes;
              return APIF_RETURN;
            }
          else
            {
          native:
              if ( SUCCEEDED(*result = NtQueryFullAttributesFile(&oa,args->FileAttributes)) )
                ; // nothing
              else if ( *result == STATUS_OBJECT_PATH_NOT_FOUND && inBox )
                *result = STATUS_OBJECT_NAME_NOT_FOUND;
              return APIF_RETURN;
            }
      }

    APIF_STATUS DoOpenFile(void *_a, unsigned *result)
      {
        NT_OPEN_FILE_ARGS *args = (NT_OPEN_FILE_ARGS*)_a;
        
        XDBG|_S*_XOr(".apif.OpenFile '%s'",20,118296327)
          %StringW(args->ObjectAttributes->ObjectName->Buffer,args->ObjectAttributes->ObjectName->Length/2);

        bool inBox = false;
        StringW internal_path;
        OA oa = args->ObjectAttributes;

        __lockon__(VFS)
          if ( FioProxyPtr p = NormalizeOAnGetProxy(&oa,internal_path,&inBox) )
            {
                
              if ( p->IsRoot() )
                {
                  *result = NtOpenFile(args->FileHandle,
                                        args->DesiredAccess,
                                        args->ObjectAttributes,
                                        args->IoStatusBlock,
                                        args->ShareAccess,
                                        args->OpenOptions);
                  if ( SUCCEEDED(*result) )
                    handles_.PutHandle(Refe(+p),*args->FileHandle);
                  return APIF_RETURN;
                }
                
              switch ( Conflict(p,&oa) )
                { 
                  case 1:
                    goto native;
                  case 2:
                    *result = STATUS_ACCESS_DENIED;
                    return APIF_RETURN;
                }

              *args->FileHandle = handles_.Put(Refe(+p));
              *result = 0;
              XLOG|_S*_XOr("<APIF/VFS> open('%s\\%s') -> %08x",33,118951725)
                      %p->PackageName() %internal_path %*args->FileHandle;
              return APIF_RETURN;
            }
          else
            {
          native:
              *result = NtOpenFile(args->FileHandle,
                                    args->DesiredAccess,
                                    //args->ObjectAttributes,
                                    &oa,
                                    args->IoStatusBlock,
                                    args->ShareAccess,
                                    args->OpenOptions);
              
              if ( *result == STATUS_OBJECT_PATH_NOT_FOUND && inBox )
                *result = STATUS_OBJECT_NAME_NOT_FOUND;
              
              return APIF_RETURN;
            }
      }

    FioProxy *GetProxy(HANDLE h)
      {
        if ( HOA::R o = handles_.Get(h) )
          return +o;
        else
          return 0;
      }

    APIF_STATUS DoCloseFile(void *_a, unsigned *result)
      {
        NT_CLOSE_ARGS *args = (NT_CLOSE_ARGS*)_a;
        XDBG|_S*_XOr(".apif.CloseFile %08x",21,122556234) % args->FileHandle;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            if (o)
              {
                __lockon__(VFS)
                  if ( o->IsValid() )
                    XLOG|_S*_XOr("<APIF/VFS> close(%08x) '%s\\%s'",31,123015023)
                            %args->FileHandle
                            %o->PackageName()
                            %o->ComposePath();
                subject_.Erase(args->FileHandle);
                handles_.Close(args->FileHandle);
                *result = 0;
              }
            else
              { *result = STATUS_INVALID_HANDLE; }
            return APIF_RETURN;
          }
        else
          return APIF_CONTINUE;
      }

    APIF_STATUS DoReadFile(void *_a, unsigned *result)
      {
        NT_READ_FILE_ARGS *args = (NT_READ_FILE_ARGS*)_a;
        XDBG|_S*_XOr(".apif.ReadFile %08x, count:%d",30,126422667) % args->FileHandle % args->Length;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            if (!o)
              *result = STATUS_INVALID_HANDLE;
            else
              __lockon__(VFS)
                {
                  unsigned a = args->ByteOffset?args->ByteOffset->LowPart:o.State();

                  if ( !o->IsValid() )
                    {
                      *result = STATUS_INVALID_HANDLE;
                      goto e;
                    }

                  if ( o->IsRoot() )
                    {
                      args->FileHandle = VFS->AppFolderHandle();
                      XDBG|_S*_XOr("<APIF/VFS> access to application folder is redirectd to handle %08x",68,128716452) % args->FileHandle;
                      return APIF_CONTINUE;
                    }

                  u32_t wasread = 0;
                  int err = o->Read(a,args->Buffer,args->Length,&wasread);
                  args->IoStatusBlock->Information = wasread;
                  if (!args->ByteOffset) o.State() += wasread;
                  if ( err )
                    if ( err == FioProxy::END_OF_FIO)
                      {
                        *result = 0; /*FIXME*/
                      }
                    else
                      {
                        *result = STATUS_INVALID_HANDLE;
                      }
                  
                  if ( args->Event )
                    SetEvent(args->Event);

                  XLOG|_S*_XOr("<APIF/VFS> read(%08x,%d,%d) '%s\\%s' -> %d,%08x",47,132386332)
                          %args->FileHandle
                          %a
                          %args->Length
                          %o->PackageName()
                          %o->ComposePath()
                          %args->IoStatusBlock->Information
                          %*result;
                }
          e:
            return APIF_RETURN;
          }
        else
          return APIF_CONTINUE;
      }

    APIF_STATUS DoWriteFile(void *_a, unsigned *result)
      {
        NT_WRITE_FILE_ARGS *args = (NT_WRITE_FILE_ARGS*)_a;
        XDBG|_S*_XOr(".apif.WriteFile %08x, count:%d",31,103353899) % args->FileHandle % args->Length;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            if (!o) *result = STATUS_INVALID_HANDLE;
            else
              __lockon__(VFS)
                {
                  if ( !o->IsValid() )
                    {
                      *result = STATUS_INVALID_HANDLE;
                      goto e;
                    }
                  if ( o->IsRoot() )
                    {
                      args->FileHandle = VFS->AppFolderHandle();
                      XDBG|_S*_XOr("<APIF/VFS> access to application folder is redirectd to handle %08x",68,105647684) % args->FileHandle;
                      return APIF_CONTINUE;
                    }
                  *result = 0;
                }
          e:
            return APIF_RETURN;
          }
        else
          return APIF_CONTINUE;
      }

    APIF_STATUS DoQueFilInfo(void *_a, unsigned *result)
      {
        NT_QUE_FILINFO_ARGS *args = (NT_QUE_FILINFO_ARGS*)_a;
        //if ( args->FileHandle ) 
        //  __asm int 3
        //  ;
        XDBG|_S*_XOr(".apif.QueFilInfo %08x, %08x",28,109316540) % args->FileHandle % args->FileInformationClass;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            if (!o) *result = STATUS_INVALID_HANDLE;
            else
              __lockon__(VFS)
                {
                  if ( !o->IsValid() )
                    {
                      *result = STATUS_INVALID_HANDLE;
                      goto e;
                    }
                  if ( o->IsRoot() )
                    {
                      args->FileHandle = VFS->AppFolderHandle();
                      XDBG|_S*_XOr("<APIF/VFS> access to application folder is redirectd to handle %08x",68,112855518) % args->FileHandle;
                      return APIF_CONTINUE;
                    }
                  *result = 0;
                  if ( args->FileInformationClass == FilePositionInformation )
                    {
                      ((FILE_POSITION_INFORMATION*)args->FileInformation)->CurrentByteOffset.LowPart = o.State();
                      ((FILE_POSITION_INFORMATION*)args->FileInformation)->CurrentByteOffset.HighPart = 0;
                    }
                  else if ( args->FileInformationClass == FileStandardInformation )
                    {
                      FILE_STANDARD_INFORMATION *i = (FILE_STANDARD_INFORMATION*)args->FileInformation;
                      memset(i,0,sizeof(FILE_STANDARD_INFORMATION));
                      i->AllocationSize.QuadPart = i->EndOfFile.QuadPart = o->Size();
                      i->Directory = o->IsDirectory();
                      i->NumberOfLinks = 1;
                    }
                  else if ( args->FileInformationClass == FileBasicInformation )
                    {
                      FILE_BASIC_INFORMATION *i = (FILE_BASIC_INFORMATION*)args->FileInformation;
                      memset(i,0,sizeof(FILE_BASIC_INFORMATION));
                      i->FileAttributes = o->IsDirectory()?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_READONLY;
                      i->LastWriteTime = o->GetMtime();
                      i->CreationTime = o->GetCtime();
                      i->LastAccessTime = i->LastWriteTime;
                      i->ChangeTime = i->LastWriteTime;
                    }
                  else if ( args->FileInformationClass == FileNetworkOpenInformation )
                    {
                      FILE_NETWORK_OPEN_INFORMATION *i = (FILE_NETWORK_OPEN_INFORMATION*)args->FileInformation;
                      memset(i,0,sizeof(FILE_NETWORK_OPEN_INFORMATION));
                      i->FileAttributes = o->IsDirectory()?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_READONLY;
                      i->LastWriteTime = o->GetMtime();
                      i->CreationTime = o->GetCtime();
                      i->LastAccessTime = i->LastWriteTime;
                      i->ChangeTime = i->LastWriteTime;
                      i->AllocationSize.QuadPart = i->EndOfFile.QuadPart = o->Size();
                    }
                  else if ( args->FileInformationClass == FileObjectIdInformation )
                    {
                      FILE_ATTRIBUTE_TAG_INFORMATION *i = (FILE_ATTRIBUTE_TAG_INFORMATION*)args->FileInformation;
                      memset(i,0,sizeof(FILE_ATTRIBUTE_TAG_INFORMATION));
                      i->FileAttributes = o->IsDirectory()?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_READONLY;
                    }
                  else if ( args->FileInformationClass == FileAllInformation )
                    {
                      FILE_ALL_INFORMATION *i = (FILE_ALL_INFORMATION*)args->FileInformation;
                      memset(i,0,sizeof(args->FileInformation));
                      FILE_BASIC_INFORMATION *j = &i->BasicInformation;
                      j->FileAttributes = o->IsDirectory()?FILE_ATTRIBUTE_DIRECTORY:FILE_ATTRIBUTE_READONLY;
                      j->LastWriteTime = o->GetMtime();
                      j->CreationTime = o->GetCtime();
                      j->LastAccessTime = j->LastWriteTime;
                      j->ChangeTime = j->LastWriteTime;
                      FILE_STANDARD_INFORMATION *k = &i->StandardInformation;
                      k->AllocationSize.QuadPart = k->EndOfFile.QuadPart = o->Size();
                      k->Directory = o->IsDirectory();
                      k->NumberOfLinks = 1;
                      FILE_NAME_INFORMATION *n = &i->NameInformation;
                      //
                    }
                  else
                    {
                      //*result = STATUS_INVALID_HANDLE;
                      args->FileHandle = VFS->SelfHandle();
                      return APIF_CONTINUE;
                    }
                }
          e:
            return APIF_RETURN;
          }
        else
          return APIF_CONTINUE;
      }

    APIF_STATUS DoSetFilInfo(void *_a, unsigned *result)
      {
        NT_SET_FILINFO_ARGS *args = (NT_SET_FILINFO_ARGS*)_a;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            if (!o) *result = STATUS_INVALID_HANDLE;
            else
              __lockon__(VFS)
                {
                  if ( !o->IsValid() )
                    {
                      *result = STATUS_INVALID_HANDLE;
                      goto e;
                    }
                  *result = 0;
                  if ( args->FileInformationClass == FilePositionInformation )
                    {
                      o.State() = ((FILE_POSITION_INFORMATION*)args->FileInformation)->CurrentByteOffset.LowPart;
                      XLOG|_S*_XOr("<APIF/VFS> seek(%08x,%d(%08x)) '%s\\%s'",39,116394256)
                          %args->FileHandle
                          %o.State()
                          %o.State()
                          %o->PackageName()
                          %o->ComposePath()
                          ;
                    }
                  else if ( args->FileInformationClass == FileDispositionInformation )
                    *result = STATUS_ACCESS_DENIED;
                  else
                    *result = 0;//STATUS_INVALID_HANDLE;
                }
          e:
            return APIF_RETURN;
          }
        else
          {
            if ( args->FileInformationClass == FileRenameInformation )
              {
                FILE_RENAME_INFORMATION *fri = (FILE_RENAME_INFORMATION*)args->FileInformation;
                __lockon__(VFS)
                  if ( HOA::R o = handles_.Get(fri->RootDirectory) )
                    {
                      if ( o->IsRoot() )
                        fri->RootDirectory = VFS->AppFolderHandle();
                      else 
                        /* FIXME */
                        // open real folder if exists 
                        ;
                    }
              }
            return APIF_CONTINUE;
          }
      }

    APIF_STATUS DoQueVolInfo(void *_a, unsigned *result)
      {
        NT_QUE_VOLINFO_ARGS *args = (NT_QUE_VOLINFO_ARGS*)_a;
        XDBG|_S*_XOr(".apif.QueVolInfo %08x, %08x",28,153946407) % args->FileHandle % args->FileSystemInformationClass;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            if (!o) *result = STATUS_INVALID_HANDLE;
            else
              __lockon__(VFS)
                {
                  if ( !o->IsValid() )
                    {
                      *result = STATUS_INVALID_HANDLE;
                      goto e;
                    }
                  if ( /*o->IsRoot()*/ o->IsDirectory() )
                    {
                      args->FileHandle = VFS->AppFolderHandle();
                      XDBG|_S*_XOr("<APIF/VFS> access to application folder is redirectd to handle %08x",68,156174661) % args->FileHandle;
                      return APIF_CONTINUE;
                    }
                  else
                    {
                      args->FileHandle = VFS->SelfHandle();
                      return APIF_CONTINUE;
                    }
                  //*result = 0;
                  //if ( args->FileSystemInformationClass == FileFsDeviceInformation )
                  //  {
                  //    ((FILE_FS_DEVICE_INFORMATION*)args->FileSystemInformation)->DeviceType = FILE_DEVICE_DISK_FILE_SYSTEM;
                  //    ((FILE_FS_DEVICE_INFORMATION*)args->FileSystemInformation)->Characteristics = 0;
                  //  }
                  //else
                  //  *result = STATUS_INVALID_HANDLE;
                }
          e:
            return APIF_RETURN;
          }
        else
          return APIF_CONTINUE;
      }

    APIF_STATUS DoQueDirFile(void *_a, unsigned *result)
      {

        NT_QUE_DIRFILE_ARGS *args = (NT_QUE_DIRFILE_ARGS*)_a;
        FILE_BOTH_DIR_INFORMATION1 *di = (FILE_BOTH_DIR_INFORMATION1*)args->FileInformation;
        XDBG|_S*_XOr(".apif.QueDirInfo %08x, %08x",28,159844541) % args->FileHandle % args->FileInformationClass;

        //__asm int 3
        ;

        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            if (!o) *result = STATUS_INVALID_HANDLE;
            else
              __lockon__(VFS)
                {
                  if ( !o->IsValid() )
                    {
                      *result = STATUS_INVALID_HANDLE;
                      goto e;
                    }

                  *result = 0;
                  args->IoStatusBlock->Status = 0;
                  args->IoStatusBlock->Information = 0;

                  //__asm int 3;
                  if ( args->FileInformationClass == FileBothDirectoryInformation
                    || args->FileInformationClass == FileOleDirectoryInformation )
                    {
                      StringW mask;
                      if ( args->FileMask )
                        mask.SetRange(args->FileMask->Buffer,args->FileMask->Length/2);

                      int idx = o.State();
                      VfsDirlistPtr ls;

                      if ( VfsDirlistPtr *_ls = subject_.Get(args->FileHandle) ) ls = *_ls;

                      //__asm int 3
                      ;

                      if ( args->RestartScan || !ls )
                        {
                          ls = VFS->ListDir(+o,mask?+mask:0);
                          XLOG|_S*_XOr("<APIF/VFS> list('<workpath>\\%s|%s')  -> %d",43,161810655)
                                  %o->ComposePath1()
                                  %mask
                                  %ls->Count();
                          subject_.Put(args->FileHandle,ls);
                          idx = 0;
                        }

                      if ( ls->Count() )
                        {
                          if ( idx >= ls->Count() )
                            {
                              *result = args->IoStatusBlock->Status
                                = args->RestartScan?STATUS_NO_SUCH_FILE:STATUS_NO_MORE_FILES;
                              goto e;
                            }

                          // ok we have found the file

                          {
                            unsigned struSz = args->FileInformationClass == FileOleDirectoryInformation 
                              ?  sizeof(FILE_ID_BOTH_DIR_INFORMATION) : sizeof(FILE_BOTH_DIR_INFORMATION);
                            unsigned nameOffs = args->FileInformationClass == FileOleDirectoryInformation 
                              ?  offset(FILE_ID_BOTH_DIR_INFORMATION,FileName) : offset(FILE_BOTH_DIR_INFORMATION,FileName);
                            memset(di,0,struSz);
                            SPACK_FILE_INFO *fio = ls->At(idx)->Get();

                            int l = struSz + fio->name.length*2+2;

                            if ( l > args->Length )
                              {
                                *result = args->IoStatusBlock->Status = STATUS_BUFFER_OVERFLOW;
                                goto e;
                              }
                            args->IoStatusBlock->Information = l;
                            
                            o.State() = idx+1;
                            di->FileIndex = idx;
                            if ( fio->flags & SVFS_DIRECTORY )
                              di->FileAttributes = FILE_ATTRIBUTE_DIRECTORY;
                            else
                              di->FileAttributes = FILE_ATTRIBUTE_NORMAL,
                              di->AllocationSize.QuadPart = di->EndOfFile.QuadPart = fio->size;
                            di->FileNameLength = fio->name.length*2;
                            if ( di->FileNameLength && fio->name.S )
                              memcpy((char*)di+nameOffs,fio->name.S,di->FileNameLength+2);
                            else
                              memset((char*)di+nameOffs,0,di->FileNameLength+2);
                            di->LastWriteTime  = ls->At(idx)->GetMtime();
                            di->CreationTime   = ls->At(idx)->GetCtime();
                            di->LastAccessTime = di->LastWriteTime;
                            di->ChangeTime     = di->LastWriteTime;
                          }
                        }
                      else
                        *result = args->IoStatusBlock->Status = STATUS_NO_SUCH_FILE;
                    }
                  else
                    {
                    //*result = STATUS_INVALID_HANDLE;
                      args->FileHandle = VFS->AppFolderHandle();
                      return APIF_CONTINUE;
                    }
                }
          e:;
            return APIF_RETURN;
          }
        else
          return APIF_CONTINUE;
      }

    APIF_STATUS DoLockFile(void *_a, unsigned *result)
      {
        NT_LOCKFILE_ARGS *args = (NT_LOCKFILE_ARGS*)_a;
        XDBG|_S*_XOr(".apif.DoLockFile %08x",22,165087469) % args->FileHandle;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
           __lockon__(VFS)
              {
                if ( !o->IsValid() )
                  {
                    *result = STATUS_INVALID_HANDLE;
                    goto e;
                  }
                *result = 0;
              }
          e:;
            return APIF_RETURN;
          }
        else
          return APIF_CONTINUE; 
      }

    APIF_STATUS DoUnlockFile(void *_a, unsigned *result)
      {
        NT_UNLOCKFILE_ARGS *args = (NT_UNLOCKFILE_ARGS*)_a;
        XDBG|_S*_XOr(".apif.DoUnlockFile %08x",24,166922257) % args->FileHandle;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            __lockon__(VFS)
              {
                if ( !o->IsValid() )
                  {
                    *result = STATUS_INVALID_HANDLE;
                    goto e;
                  }
                *result = 0;
              }
          e:;
            return APIF_RETURN;
          }
        else
          return APIF_CONTINUE; 
      }
      
    APIF_STATUS DoGetEnVar(void *args, unsigned *result)
      {
        struct GEV { LPCWSTR lpName;LPWSTR lpBuffer;DWORD nSize; } *a = (GEV*)args;
        StringA name = a->lpName;
        if ( StrSafeEqualI(+name,_XOr(";molebox;;exefilter;",21,136251445)) )
          {
            *result = (unsigned)&CoreAPI_EnableExeFilter;
            return APIF_RETURN;
          }
        else if ( StrSafeEqualI(+name,_XOr(";molebox;;fsfilter;",20,136710178)) )
          {
            *result = (unsigned)&CoreAPI_EnableFsFilter;
            return APIF_RETURN;
          }
        else if ( StrSafeEqualI(+name,_XOr(";molebox;;mount;",17,139331658)) )
          {
            *result = (unsigned)&CoreAPI_Mount;
            return APIF_RETURN;
          }
        else if ( StrSafeEqualI(+name,_XOr(";molebox;;unmount;",19,139462768)) )
          {
            *result = (unsigned)&CoreAPI_Unmount;
            return APIF_RETURN;
          }
        else if ( StrSafeEqualI(+name,_XOr(";molebox;;coreapi;",19,140839839)) )
          {
            if ( a->nSize >= sizeof(COREAPI) )
              {
                COREAPI *api = (COREAPI*)a->lpBuffer;
                api->EnableExeFilter = &CoreAPI_EnableExeFilter;
                api->EnableFsFilter = &CoreAPI_EnableFsFilter;
                api->Mount = &CoreAPI_Mount;
                api->Unmount = &CoreAPI_Unmount;
              }
            return APIF_CONTINUE;
          }   
        return APIF_CONTINUE;
      }
      
    APIF_STATUS DoDuplicate(void *_a, unsigned *result)
      {
        NT_DUPLICATE_ARGS *args = (NT_DUPLICATE_ARGS *)_a;
        XDBG|_S*_XOr(".apif.DuplicateFile %08x",25,142740358) %args->SourceHandle;
        if ( HOA::R o = handles_.Get(args->SourceHandle) )
          {
            __lockon__(VFS)
              {
                *args->TargetHandle = handles_.Put(Refe(+o));
                if ( o->IsValid() )
                  XLOG|_S*_XOr("<APIF/RGS> duplicate file (%08x) '%s' -> %08x",46,145492900)
                          %args->SourceHandle
                          %o->ComposePath()
                          %*args->TargetHandle;
                *result = 0;
              }
            return APIF_RETURN;
          }
        return APIF_CONTINUE;
      };
      
    APIF_STATUS DoNotifChangeFile(void *_a, unsigned *result)
      {
        NT_NOTIFCHGF_ARGS *args = (NT_NOTIFCHGF_ARGS *)_a;
        XDBG|_S*_XOr(".apif.NotifChangeFile %08x",27,148638708) %args->FileHandle;
        if ( HOA::R o = handles_.Get(args->FileHandle) )
          {
            //__asm int 3;
            __lockon__(VFS)
              {
                HANDLE newH = 0;
                if ( o->IsRoot() )
                  newH = VFS->AppFolderHandle();
                if ( o->IsValid() )
                  XLOG|_S*_XOr("<APIF/RGS> query changes on (%08x) '%s\\%s' -> %08x",51,150932245)
                          %args->FileHandle
                          %o->PackageName()
                          %o->ComposePath()
                          %newH;
                //if ( newH )
                //  {
                //    args->FileHandle = newH;
                //    return APIF_CONTINUE;
                //  }
                *result = STATUS_PENDING;
                if ( args->Event ) SetEvent(args->Event);
                return APIF_RETURN;
              }
          }
        return APIF_CONTINUE;
      }
      
    virtual APIF_STATUS DoCall(int apif_id, void *args, unsigned *result)
      {
        if ( !fsFilterIsEnabled && apif_id != APIF_GETENVAR) return APIF_CONTINUE;
        
        switch(apif_id)
          {
            case APIF_CREATEFILE:
              return DoCreateFile(args,result);
            case APIF_OPENFILE:
              return DoOpenFile(args,result);
            case APIF_READFILE:
              return DoReadFile(args,result);
            case APIF_WRITEFILE:
              return DoWriteFile(args,result);
            case APIF_CLOSE:
              return DoCloseFile(args,result);
            case APIF_QUEFILINFO:
              return DoQueFilInfo(args,result);
            case APIF_SETFILINFO:
              return DoSetFilInfo(args,result);
            case APIF_QUEVOLINFO:
              return DoQueVolInfo(args,result);
            case APIF_QUEDIRFILE:
              return DoQueDirFile(args,result);
            case APIF_QUEFATTR:
              return DoQueFileAttr(args,result);
            case APIF_QUEFATTRFULL:
              return DoQueFileAttrFull(args,result);
            case APIF_LOCKFILE:
              return DoLockFile(args,result);
            case APIF_UNLOCKFILE:
              return DoUnlockFile(args,result);
            case APIF_GETENVAR:
              return DoGetEnVar(args,result);
            case APIF_DUPLICATE:
              return DoDuplicate(args,result);
            case APIF_NOTICHNGFL:
              return DoNotifChangeFile(args,result);
          }
        return APIF_CONTINUE;
      }

    FileObjectFilter(FileObjectFilter const&);
  };

FileObjectFilter apif_fobjfilt = FileObjectFilter();

void APIF_RegisterFileObjectHooks()
  {
    APIF->Push(&apif_fobjfilt);
  };

FioProxy *GetProxyOfApifFileHandle(HANDLE h)
  {
    return apif_fobjfilt.GetProxy(h);
  }
