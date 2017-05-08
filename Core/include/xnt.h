
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#ifndef ___5fdbd7d5_bf57_413b_b33b_35688479e703___
#define ___5fdbd7d5_bf57_413b_b33b_35688479e703___

#define NT_HEADERS(x) ((IMAGE_NT_HEADERS*)((unsigned char*)x + ((IMAGE_DOS_HEADER*)x)->e_lfanew))

#ifndef STATUS_ACCESS_DENIED
#define STATUS_ACCESS_DENIED          ((NTSTATUS)0xC0000022L)
#endif

#ifndef STATUS_OBJECT_PATH_NOT_FOUND
#define STATUS_OBJECT_PATH_NOT_FOUND  ((NTSTATUS)0xC000003AL)
#endif

#ifndef STATUS_OBJECT_NAME_COLLISION
#define STATUS_OBJECT_NAME_COLLISION ((NTSTATUS)0xC0000035L)
#endif

#ifndef FILE_OPEN
#define FILE_OPEN                       0x00000001
#define FILE_CREATE                     0x00000002
#endif

typedef enum _FS_INFORMATION_CLASS
{
    FileFsVolumeInformation = 1,
    FileFsLabelInformation,
    FileFsSizeInformation,
    FileFsDeviceInformation,
    FileFsAttributeInformation,
    FileFsControlInformation,
    FileFsFullSizeInformation,
    FileFsObjectIdInformation,
    FileFsMaximumInformation
} FS_INFORMATION_CLASS, *PFS_INFORMATION_CLASS;

typedef enum _XFILE_INFORMATION_CLASS
{
    _FileDirectoryInformation = 1,    // conflicts with winternl.h
    FileFullDirectoryInformation,     // 2
    FileBothDirectoryInformation,     // 3
    FileBasicInformation,             // 4
    FileStandardInformation,          // 5
    FileInternalInformation,          // 6
    FileEaInformation,                // 7
    FileAccessInformation,            // 8
    FileNameInformation,              // 9
    FileRenameInformation,            // 10
    FileLinkInformation,              // 11
    FileNamesInformation,             // 12
    FileDispositionInformation,       // 13
    FilePositionInformation,          // 14
    FileFullEaInformation,            // 15
    FileModeInformation,              // 16
    FileAlignmentInformation,         // 17
    FileAllInformation,               // 18
    FileAllocationInformation,        // 19
    FileEndOfFileInformation,         // 20
    FileAlternateNameInformation,     // 21
    FileStreamInformation,            // 22
    FilePipeInformation,              // 23
    FilePipeLocalInformation,         // 24
    FilePipeRemoteInformation,        // 25
    FileMailslotQueryInformation,     // 26
    FileMailslotSetInformation,       // 27
    FileCompressionInformation,       // 28
    FileCopyOnWriteInformation,       // 29
    FileCompletionInformation,        // 30
    FileMoveClusterInformation,       // 31
    FileOleClassIdInformation,        // 32
    FileOleStateBitsInformation,      // 33
    FileNetworkOpenInformation,       // 34
    FileObjectIdInformation,          // 35
    FileOleAllInformation,            // 36
    FileOleDirectoryInformation,      // 37
    FileContentIndexInformation,      // 38
    FileInheritContentIndexInformation,// 39
    FileOleInformation,               // 40
    FileMaximumInformation            // 41
} XFILE_INFORMATION_CLASS;

typedef enum _SECTION_INHERIT
{
    ViewShare = 1,
    ViewUnmap = 2
} SECTION_INHERIT, *PSECTION_INHERIT;

typedef enum _SECTION_INFORMATION_CLASS
{
    SectionBasicInformation,
    SectionImageInformation
} SECTION_INFORMATION_CLASS;

typedef struct _SECTIONBASICINFO
{
    PVOID BaseAddress;
    ULONG AllocationAttributes;
    LARGE_INTEGER MaximumSize;
} SECTION_BASIC_INFORMATION, *PSECTION_BASIC_INFORMATION;

typedef struct _SECTION_IMAGE_INFORMATION
{
    PVOID TransferAddress;
    ULONG ZeroBits;
    ULONG MaximumStackSize;
    ULONG CommittedStackSize;
    ULONG SubSystemType;
    union
    {
        struct
        {
            USHORT SubSystemMinorVersion;
            USHORT SubSystemMajorVersion;
        };
        ULONG SubSystemVersion;
    };
    ULONG GpValue;
    USHORT ImageCharacteristics;
    USHORT DllCharacteristics;
    USHORT Machine;
    BOOLEAN ImageContainsCode;
    BOOLEAN Spare1;
    ULONG LoaderFlags;
    ULONG Reserved[ 2 ];
} SECTION_IMAGE_INFORMATION, *PSECTION_IMAGE_INFORMATION;

typedef struct _FILE_FULL_EA_INFORMATION
{
    ULONG                   NextEntryOffset;
    BYTE                    Flags;
    BYTE                    EaNameLength;
    USHORT                  EaValueLength;
    CHAR                    EaName[1];
} FILE_FULL_EA_INFORMATION, *PFILE_FULL_EA_INFORMATION;

typedef struct _FILE_FS_DEVICE_INFORMATION
{
    ULONG DeviceType;
    ULONG Characteristics;
} FILE_FS_DEVICE_INFORMATION, *PFILE_FS_DEVICE_INFORMATION;

typedef struct _FILE_STANDARD_INFORMATION
{
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG NumberOfLinks;
    BOOLEAN DeletePending;
    BOOLEAN Directory;
} FILE_STANDARD_INFORMATION, *PFILE_STANDARD_INFORMATION;

typedef struct _FILE_NETWORK_OPEN_INFORMATION
{
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER AllocationSize;
    LARGE_INTEGER EndOfFile;
    ULONG FileAttributes;
    ULONG Unknown;
} FILE_NETWORK_OPEN_INFORMATION, *PFILE_NETWORK_OPEN_INFORMATION;


typedef struct _FILE_POSITION_INFORMATION
{
    LARGE_INTEGER CurrentByteOffset;
} FILE_POSITION_INFORMATION, *PFILE_POSITION_INFORMATION;

typedef struct _FILE_BOTH_DIR_INFORMATION1
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
} FILE_BOTH_DIR_INFORMATION1;

typedef struct _FILE_BOTH_DIR_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    WCHAR FileName[1];
} FILE_BOTH_DIR_INFORMATION, *PFILE_BOTH_DIR_INFORMATION;

typedef struct _FILE_ID_BOTH_DIR_INFORMATION
{
    ULONG NextEntryOffset;
    ULONG FileIndex;
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    LARGE_INTEGER EndOfFile;
    LARGE_INTEGER AllocationSize;
    ULONG FileAttributes;
    ULONG FileNameLength;
    ULONG EaSize;
    CCHAR ShortNameLength;
    WCHAR ShortName[12];
    LARGE_INTEGER FileId;
    WCHAR FileName[1];
} FILE_ID_BOTH_DIR_INFORMATION, *PFILE_ID_BOTH_DIR_INFORMATION;

typedef struct _FILE_BASIC_INFORMATION
{
    LARGE_INTEGER CreationTime;
    LARGE_INTEGER LastAccessTime;
    LARGE_INTEGER LastWriteTime;
    LARGE_INTEGER ChangeTime;
    ULONG FileAttributes;
} FILE_BASIC_INFORMATION, *PFILE_BASIC_INFORMATION;

typedef struct _FILE_ATTRIBUTE_TAG_INFORMATION
{
    ULONG FileAttributes;
    ULONG ReparseTag;
} FILE_ATTRIBUTE_TAG_INFORMATION, *PFILE_ATTRIBUTE_TAG_INFORMATION;

typedef struct _FILE_ACCESS_INFORMATION
{
    ACCESS_MASK AccessFlags;
} FILE_ACCESS_INFORMATION, *PFILE_ACCESS_INFORMATION;

typedef struct _FILE_MODE_INFORMATION
{
    ULONG Mode;
} FILE_MODE_INFORMATION, *PFILE_MODE_INFORMATION;

typedef struct _FILE_EA_INFORMATION
{
    ULONG EaSize;
} FILE_EA_INFORMATION, *PFILE_EA_INFORMATION;

typedef struct _FILE_INTERNAL_INFORMATION
{
    LARGE_INTEGER IndexNumber;
} FILE_INTERNAL_INFORMATION, *PFILE_INTERNAL_INFORMATION;

typedef struct _FILE_ALIGNMENT_INFORMATION
{
    ULONG AlignmentRequirement;
} FILE_ALIGNMENT_INFORMATION, *PFILE_ALIGNMENT_INFORMATION;

typedef struct _FILE_NAME_INFORMATION
{
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_NAME_INFORMATION, *PFILE_NAME_INFORMATION;

typedef struct _FILE_ALL_INFORMATION
{
    FILE_BASIC_INFORMATION     BasicInformation;
    FILE_STANDARD_INFORMATION  StandardInformation;
    FILE_INTERNAL_INFORMATION  InternalInformation;
    FILE_EA_INFORMATION        EaInformation;
    FILE_ACCESS_INFORMATION    AccessInformation;
    FILE_POSITION_INFORMATION  PositionInformation;
    FILE_MODE_INFORMATION      ModeInformation;
    FILE_ALIGNMENT_INFORMATION AlignmentInformation;
    FILE_NAME_INFORMATION      NameInformation;
} FILE_ALL_INFORMATION, *PFILE_ALL_INFORMATION;

#define FILE_DEVICE_DISK_FILE_SYSTEM  0x00000008
#define STATUS_NO_SUCH_FILE           ((NTSTATUS)0xC000000FL)
#define STATUS_BUFFER_OVERFLOW        ((NTSTATUS)0x80000005L)
#define STATUS_NO_MORE_FILES          ((NTSTATUS)0x80000006L)
#define STATUS_CONFLICTING_ADDRESSES  ((NTSTATUS)0xC0000018L)
#define STATUS_IMAGE_NOT_AT_BASE      ((NTSTATUS)0x40000003L)
#define STATUS_OBJECT_NAME_NOT_FOUND  ((NTSTATUS)0xC0000034L)
#ifndef STATUS_INVALID_PARAMETER
#define STATUS_INVALID_PARAMETER      ((NTSTATUS)0xC000000DL)
#endif
#define STATUS_NO_MORE_ENTRIES        ((NTSTATUS)0x8000001AL)
#define STATUS_BUFFER_TOO_SMALL       ((NTSTATUS)0xC0000023L)

#ifndef NT_SUCCESS
#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)
#define NT_INFORMATION(Status) ((ULONG)(Status) >> 30 == 1)
#define NT_WARNING(Status) ((ULONG)(Status) >> 30 == 2)
#define NT_ERROR(Status) ((ULONG)(Status) >> 30 == 3)
#endif

#define InitializeObjectAttributes( p, n, a, r, s ) { \
        (p)->Length = sizeof( OBJECT_ATTRIBUTES );          \
        (p)->RootDirectory = r;                             \
        (p)->Attributes = a;                                \
        (p)->ObjectName = n;                                \
        (p)->SecurityDescriptor = s;                        \
        (p)->SecurityQualityOfService = NULL;               \
    }

#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#ifndef OBJ_VALID_ATTRIBUTES
#define OBJ_VALID_ATTRIBUTES    0x000001F2L
#endif

typedef enum _KEY_INFORMATION_CLASS
{
    KeyBasicInformation,
    KeyNodeInformation,
    KeyFullInformation,
    KeyNameInformation,
    KeyCachedInformation,
    KeyFlagsInformation,
    KeyVirtualizationInformation,
    KeyHandleTagsInformation,
    MaxKeyInfoClass  // MaxKeyInfoClass should always be the last enum
} KEY_INFORMATION_CLASS;

typedef enum _KEY_SET_INFORMATION_CLASS
{
    KeyWriteTimeInformation,
    KeyWow64FlagsInformation,
    KeyControlFlagsInformation,
    KeySetVirtualizationInformation,
    KeySetDebugInformation,
    KeySetHandleTagsInformation,
    MaxKeySetInfoClass  // MaxKeySetInfoClass should always be the last enum
} KEY_SET_INFORMATION_CLASS;

typedef struct _KEY_BASIC_INFORMATION
{
    LARGE_INTEGER LastWriteTime;
    ULONG  TitleIndex;
    ULONG  NameLength;
    WCHAR  Name[1];  //  Variable-length string
} KEY_BASIC_INFORMATION, *PKEY_BASIC_INFORMATION;

typedef struct _KEY_NAME_INFORMATION
{
    ULONG  NameLength;
    WCHAR  Name[1];  //  Variable-length string
} KEY_NAME_INFORMATION, *PKEY_NAME_INFORMATION;

typedef struct _KEY_NODE_INFORMATION
{
    LARGE_INTEGER LastWriteTime;
    ULONG  TitleIndex;
    ULONG  ClassOffset;
    ULONG  ClassLength;
    ULONG  NameLength;
    WCHAR  Name[1];  //  Variable-length string
} KEY_NODE_INFORMATION, *PKEY_NODE_INFORMATION;

typedef struct _KEY_FULL_INFORMATION
{
    LARGE_INTEGER  LastWriteTime;
    ULONG  TitleIndex;
    ULONG  ClassOffset;
    ULONG  ClassLength;
    ULONG  SubKeys;
    ULONG  MaxNameLen;
    ULONG  MaxClassLen;
    ULONG  Values;
    ULONG  MaxValueNameLen;
    ULONG  MaxValueDataLen;
    WCHAR  Class[1];
} KEY_FULL_INFORMATION, *PKEY_FULL_INFORMATION;

typedef struct _KEY_CACHED_INFORMATION
{
    LARGE_INTEGER LastWriteTime;
    ULONG   TitleIndex;
    ULONG   SubKeys;
    ULONG   MaxNameLen;
    ULONG   Values;
    ULONG   MaxValueNameLen;
    ULONG   MaxValueDataLen;
    ULONG   NameLength;
    WCHAR   Name[1];            // Variable length string
} KEY_CACHED_INFORMATION, *PKEY_CACHED_INFORMATION;

typedef enum MY_OBJECT_INFORMATION_CLASS
{
    MyObjectBasicInformation,     // Result is OBJECT_BASIC_INFORMATION structure
    MyObjectNameInformation,      // Result is OBJECT_NAME_INFORMATION structure
    MyObjectTypeInformation,      // Result is OBJECT_TYPE_INFORMATION structure
    MyObjectAllInformation,       // Result is OBJECT_ALL_INFORMATION structure
    MyObjectDataInformation       // Result is OBJECT_DATA_INFORMATION structure
} MY_OBJECT_INFORMATION_CLASS;

typedef struct _OBJECT_NAME_INFORMATION
{
    UNICODE_STRING ObjectName;
} OBJECT_NAME_INFORMATION, *POBJECT_NAME_INFORMATION;

typedef enum _KEY_VALUE_INFORMATION_CLASS
{
    KeyValueBasicInformation,
    KeyValueFullInformation,
    KeyValuePartialInformation
} KEY_VALUE_INFORMATION_CLASS;

typedef struct _KEY_VALUE_BASIC_INFORMATION
{
    ULONG  TitleIndex;
    ULONG  Type;
    ULONG  NameLength;
    WCHAR  Name[1];  //  Variable size
} KEY_VALUE_BASIC_INFORMATION, *PKEY_VALUE_BASIC_INFORMATION;

typedef struct _KEY_VALUE_FULL_INFORMATION
{
    ULONG  TitleIndex;
    ULONG  Type;
    ULONG  DataOffset;
    ULONG  DataLength;
    ULONG  NameLength;
    WCHAR  Name[1];  //  Variable size
} KEY_VALUE_FULL_INFORMATION, *PKEY_VALUE_FULL_INFORMATION;

typedef struct _KEY_VALUE_PARTIAL_INFORMATION
{
    ULONG  TitleIndex;
    ULONG  Type;
    ULONG  DataLength;
    UCHAR  Data[1];  //  Variable size
} KEY_VALUE_PARTIAL_INFORMATION, *PKEY_VALUE_PARTIAL_INFORMATION;


typedef struct _KEY_WOW64_FLAGS_INFORMATION
{
    ULONG   UserFlags;
} KEY_WOW64_FLAGS_INFORMATION, *PKEY_WOW64_FLAGS_INFORMATION;

typedef struct _KEY_HANDLE_TAGS_INFORMATION
{
    ULONG   HandleTags;
} KEY_HANDLE_TAGS_INFORMATION, *PKEY_HANDLE_TAGS_INFORMATION;

typedef struct _KEY_CONTROL_FLAGS_INFORMATION
{
    ULONG   ControlFlags;
} KEY_CONTROL_FLAGS_INFORMATION, *PKEY_CONTROL_FLAGS_INFORMATION;

typedef struct _KEY_SET_VIRTUALIZATION_INFORMATION
{
    ULONG   VirtualTarget           : 1; // Tells if the key is a virtual target key.
    ULONG   VirtualStore            : 1; // Tells if the key is a virtual store key.
    ULONG   VirtualSource           : 1; // Tells if the key has been virtualized at least one (virtual hint)
    ULONG   Reserved                : 29;
} KEY_SET_VIRTUALIZATION_INFORMATION, *PKEY_SET_VIRTUALIZATION_INFORMATION;

struct IStream_Vtbl
{
    HRESULT(STDMETHODCALLTYPE* QueryInterface)(void* This, REFIID riid, void** ppvObject);
    ULONG(STDMETHODCALLTYPE* AddRef)(void* This);
    ULONG(STDMETHODCALLTYPE* Release)(void* This);
    HRESULT(STDMETHODCALLTYPE* Read)(void* This, void* pv, ULONG cb, ULONG* pcbRead);
    HRESULT(STDMETHODCALLTYPE* Write)(void* This, const void* pv, ULONG cb, ULONG* pcbWritten);
    HRESULT(STDMETHODCALLTYPE* Seek)(void* This, LARGE_INTEGER dlibMove, DWORD dwOrigin, ULARGE_INTEGER* plibNewPosition);
    HRESULT(STDMETHODCALLTYPE* SetSize)(void* This, ULARGE_INTEGER libNewSize);
    HRESULT(STDMETHODCALLTYPE* CopyTo)(void* This, IStream* pstm, ULARGE_INTEGER cb, ULARGE_INTEGER* pcbRead,
                                       ULARGE_INTEGER* pcbWritten);
    HRESULT(STDMETHODCALLTYPE* Commit)(void* This, DWORD grfCommitFlags);
    HRESULT(STDMETHODCALLTYPE* Revert)(void* This);
    HRESULT(STDMETHODCALLTYPE* LockRegion)(void* This, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    HRESULT(STDMETHODCALLTYPE* UnlockRegion)(void* This, ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType);
    HRESULT(STDMETHODCALLTYPE* Stat)(void* This, STATSTG* pstatstg, DWORD grfStatFlag);
    HRESULT(STDMETHODCALLTYPE* Clone)(void* This, IStream** ppstm);
};

typedef struct _FILE_RENAME_INFORMATION
{
    BOOLEAN ReplaceIfExists;
    HANDLE RootDirectory;
    ULONG FileNameLength;
    WCHAR FileName[1];
} FILE_RENAME_INFORMATION, *PFILE_RENAME_INFORMATION;

#define offset(S,f) ((int)&(((S*)0)->f))

#pragma pack(push,2)

typedef struct _XSYSTEM_BASIC_INFORMATION
{
    ULONG Reserved;
    ULONG TimerResolution;
    ULONG PageSize;
    ULONG NumberOfPhysicalPages;
    ULONG LowestPhysicalPageNumber;
    ULONG HighestPhysicalPageNumber;
    ULONG AllocationGranularity;
    ULONG MinimumUserModeAddress;
    ULONG MaximumUserModeAddress;
    KAFFINITY ActiveProcessorsAffinityMask;
    CCHAR NumberOfProcessors;
} XSYSTEM_BASIC_INFORMATION, *PXSYSTEM_BASIC_INFORMATION;

typedef struct _BASE_STATIC_SERVER_DATA
{
    UNICODE_STRING WindowsDirectory;
    UNICODE_STRING WindowsSystemDirectory;
    UNICODE_STRING NamedObjectDirectory;
    USHORT WindowsMajorVersion;
    USHORT WindowsMinorVersion;
    USHORT BuildNumber;
    WCHAR CSDVersion[ 131 ];
    XSYSTEM_BASIC_INFORMATION SysInfo;
    SYSTEM_TIMEOFDAY_INFORMATION TimeOfDay;
    //PINIFILE_MAPPING IniFileMapping;
    //NLS_USER_INFO NlsUserInfo;
    //BOOLEAN DefaultSeparateVDM;
    //BOOLEAN Wx86Enabled;
} BASE_STATIC_SERVER_DATA, *PBASE_STATIC_SERVER_DATA;

#pragma pack(pop)

#define DIRECTORY_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED | 0xF)
#define NtCurrentPeb() (((PXTEB)_TIB())->Peb)

typedef struct MY_PEB_LDR_DATA
{
    ULONG                   Length;
    BOOLEAN                 Initialized;
    PVOID                   SsHandle;
    LIST_ENTRY              InLoadOrderModuleList;
    LIST_ENTRY              InMemoryOrderModuleList;
    LIST_ENTRY              InInitializationOrderModuleList;
} MY_PEB_LDR_DATA;

typedef struct _LDR_MODULE
{
    LIST_ENTRY ModuleList;
    PVOID a, b;
    PVOID BaseAddress;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    SHORT LoadCount;
    SHORT TlsIndex;
    LIST_ENTRY HashTableEntry;
    ULONG TimeDateStamp;
} LDR_MODULE, *PLDR_MODULE;

typedef struct _PEB_FREE_BLOCK
{
    struct _PEB_FREE_BLOCK* Next;
    ULONG                   Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

typedef struct _RTL_DRIVE_LETTER_CURDIR
{
    USHORT                  Flags;
    USHORT                  Length;
    ULONG                   TimeStamp;
    UNICODE_STRING          DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct MY_RTL_USER_PROCESS_PARAMETERS
{
    ULONG                   MaximumLength;
    ULONG                   Length;
    ULONG                   Flags;
    ULONG                   DebugFlags;
    PVOID                   ConsoleHandle;
    ULONG                   ConsoleFlags;
    HANDLE                  StdInputHandle;
    HANDLE                  StdOutputHandle;
    HANDLE                  StdErrorHandle;
    UNICODE_STRING          CurrentDirectoryPath;
    HANDLE                  CurrentDirectoryHandle;
    UNICODE_STRING          DllPath;
    UNICODE_STRING          ImagePathName;
    UNICODE_STRING          CommandLine;
    PVOID                   Environment;
    ULONG                   StartingPositionLeft;
    ULONG                   StartingPositionTop;
    ULONG                   Width;
    ULONG                   Height;
    ULONG                   CharWidth;
    ULONG                   CharHeight;
    ULONG                   ConsoleTextAttributes;
    ULONG                   WindowFlags;
    ULONG                   ShowWindowFlags;
    UNICODE_STRING          WindowTitle;
    UNICODE_STRING          DesktopInfo;
    UNICODE_STRING          ShellInfo;
    UNICODE_STRING          RuntimeData;
    RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
} MY_RTL_USER_PROCESS_PARAMETERS;

typedef struct _XPEB
{
    BOOLEAN                 InheritedAddressSpace;
    BOOLEAN                 ReadImageFileExecOptions;
    BOOLEAN                 BeingDebugged;
    BOOLEAN                 Spare;
    HANDLE                  Mutant;
    PVOID                   ImageBaseAddress;
    PPEB_LDR_DATA           LoaderData;
    MY_RTL_USER_PROCESS_PARAMETERS* ProcessParameters;
    PVOID                   SubSystemData;
    PVOID                   ProcessHeap;
    PVOID                   FastPebLock;
    /*PPEBLOCKROUTINE*/ void* FastPebLockRoutine;
    /*PPEBLOCKROUTINE*/ void* FastPebUnlockRoutine;
    ULONG                   EnvironmentUpdateCount;
    PVOID*                   KernelCallbackTable;
    PVOID                   EventLogSection;
    PVOID                   EventLog;
    PPEB_FREE_BLOCK         FreeList;
    ULONG                   TlsExpansionCounter;
    PVOID                   TlsBitmap;
    ULONG                   TlsBitmapBits[0x2];
    PVOID                   ReadOnlySharedMemoryBase;
    PVOID                   ReadOnlySharedMemoryHeap;
    BASE_STATIC_SERVER_DATA** ReadOnlyStaticServerData;
    PVOID                   AnsiCodePageData;
    PVOID                   OemCodePageData;
    PVOID                   UnicodeCaseTableData;
    ULONG                   NumberOfProcessors;
    ULONG                   NtGlobalFlag;
    BYTE                    Spare2[0x4];
    LARGE_INTEGER           CriticalSectionTimeout;
    ULONG                   HeapSegmentReserve;
    ULONG                   HeapSegmentCommit;
    ULONG                   HeapDeCommitTotalFreeThreshold;
    ULONG                   HeapDeCommitFreeBlockThreshold;
    ULONG                   NumberOfHeaps;
    ULONG                   MaximumNumberOfHeaps;
    PVOID**                   ProcessHeaps;
    PVOID                   GdiSharedHandleTable;
    PVOID                   ProcessStarterHelper;
    PVOID                   GdiDCAttributeList;
    PVOID                   LoaderLock;
    ULONG                   OSMajorVersion;
    ULONG                   OSMinorVersion;
    ULONG                   OSBuildNumber;
    ULONG                   OSPlatformId;
    ULONG                   ImageSubSystem;
    ULONG                   ImageSubSystemMajorVersion;
    ULONG                   ImageSubSystemMinorVersion;
    ULONG                   GdiHandleBuffer[0x22];
    ULONG                   PostProcessInitRoutine;
    ULONG                   TlsExpansionBitmap;
    BYTE                    TlsExpansionBitmapBits[0x80];
    ULONG                   SessionId;
} XPEB, *PXPEB;

typedef struct _CLIENT_ID { PVOID UniqueProcess; PVOID UniqueThread; } CLIENT_ID , *PCLIENT_ID;

typedef struct _XTEB
{
    NT_TIB                  Tib;
    PVOID                   EnvironmentPointer;
    CLIENT_ID               ClientId;
    PVOID                   ActiveRpcInfo;
    PVOID                   ThreadLocalStoragePointer;
    PXPEB                   Peb;
    ULONG                   LastErrorValue;
    ULONG                   CountOfOwnedCriticalSections;
    PVOID                   CsrClientThread;
    PVOID                   Win32ThreadInfo;
    ULONG                   Win32ClientInfo[0x1F];
    PVOID                   WOW32Reserved;
    ULONG                   CurrentLocale;
    ULONG                   FpSoftwareStatusRegister;
    PVOID                   SystemReserved1[0x36];
    PVOID                   Spare1;
    ULONG                   ExceptionCode;
    ULONG                   SpareBytes1[0x28];
    PVOID                   SystemReserved2[0xA];
    ULONG                   GdiRgn;
    ULONG                   GdiPen;
    ULONG                   GdiBrush;
    CLIENT_ID               RealClientId;
    PVOID                   GdiCachedProcessHandle;
    ULONG                   GdiClientPID;
    ULONG                   GdiClientTID;
    PVOID                   GdiThreadLocaleInfo;
    PVOID                   UserReserved[5];
    PVOID                   GlDispatchTable[0x118];
    ULONG                   GlReserved1[0x1A];
    PVOID                   GlReserved2;
    PVOID                   GlSectionInfo;
    PVOID                   GlSection;
    PVOID                   GlTable;
    PVOID                   GlCurrentRC;
    PVOID                   GlContext;
    NTSTATUS                LastStatusValue;
    UNICODE_STRING          StaticUnicodeString;
    WCHAR                   StaticUnicodeBuffer[0x105];
    PVOID                   DeallocationStack;
    PVOID                   TlsSlots[0x40];
    LIST_ENTRY              TlsLinks;
    PVOID                   Vdm;
    PVOID                   ReservedForNtRpc;
    PVOID                   DbgSsReserved[0x2];
    ULONG                   HardErrorDisabled;
    PVOID                   Instrumentation[0x10];
    PVOID                   WinSockData;
    ULONG                   GdiBatchCount;
    ULONG                   Spare2;
    ULONG                   Spare3;
    ULONG                   Spare4;
    PVOID                   ReservedForOle;
    ULONG                   WaitingOnLoaderLock;
    PVOID                   StackCommit;
    PVOID                   StackCommitMax;
    PVOID                   StackReserved;
} XTEB, *PXTEB;

typedef struct _INITIAL_TEB
{
    struct
    {
        PVOID OldStackBase;
        PVOID OldStackLimit;
    } OldInitialTeb;
    PVOID StackBase;
    PVOID StackLimit;
    PVOID StackAllocationBase;
    //PVOID                StackBase;
    //PVOID                StackLimit;
    //PVOID                StackCommit;
    //PVOID                StackCommitMax;
    //PVOID                StackReserved;
} INITIAL_TEB, *PINITIAL_TEB;

#define CSR_MAKE_API_NUMBER( DllIndex, ApiIndex ) \
    (CSR_API_NUMBER)(((DllIndex) << 16) | (ApiIndex))

typedef struct _CSR_API_CONNECTINFO
{
    OUT HANDLE ObjectDirectory;
    OUT PVOID SharedSectionBase;
    OUT PVOID SharedStaticServerData;
    OUT PVOID SharedSectionHeap;
    OUT ULONG DebugFlags;
    OUT ULONG SizeOfPebData;
    OUT ULONG SizeOfTebData;
    OUT ULONG NumberOfServerDllNames;
    OUT HANDLE ServerProcessId;
} CSR_API_CONNECTINFO, *PCSR_API_CONNECTINFO;

typedef struct _CSR_CLIENTCONNECT_MSG
{
    IN ULONG ServerDllIndex;
    IN OUT PVOID ConnectionInformation;
    IN OUT ULONG ConnectionInformationLength;
} CSR_CLIENTCONNECT_MSG, *PCSR_CLIENTCONNECT_MSG;

#define CSR_NORMAL_PRIORITY_CLASS   0x00000010
#define CSR_IDLE_PRIORITY_CLASS     0x00000020
#define CSR_HIGH_PRIORITY_CLASS     0x00000040
#define CSR_REALTIME_PRIORITY_CLASS 0x00000080

struct _CSR_CAPTURE_HEADER;
typedef struct _CSR_CAPTURE_HEADER CSR_CAPTURE_HEADER, *PCSR_CAPTURE_HEADER;

typedef struct _CSR_CAPTURE_HEADER
{
    ULONG Length;
    PCSR_CAPTURE_HEADER RelatedCaptureBuffer;
    ULONG CountMessagePointers;
    PCHAR FreeSpace;
    ULONG_PTR MessagePointerOffsets[1]; // Offsets within CSR_API_MSG of pointers
} CSR_CAPTURE_HEADER, *PCSR_CAPTURE_HEADER;

typedef ULONG CSR_API_NUMBER;

typedef struct _PORT_MESSAGE
{
    union
    {
        struct
        {
            SHORT DataLength;
            SHORT TotalLength;
        } s1;
        ULONG Length;
    } u1;
    union
    {
        struct
        {
            SHORT Type;
            SHORT DataInfoOffset;
        } s2;
        ULONG ZeroInit;
    } u2;
    union
    {
        CLIENT_ID ClientId;
        double DoNotUseThisField;       // Force quadword alignment
    };
    ULONG MessageId;
    union
    {
        ULONG ClientViewSize;               // Only valid on LPC_CONNECTION_REQUEST message
        ULONG CallbackId;                   // Only valid on LPC_REQUEST message
    };
    //  UCHAR Data[];
} PORT_MESSAGE, *PPORT_MESSAGE;


typedef struct _CSR_API_MSG
{
    PORT_MESSAGE h;
    union
    {
        CSR_API_CONNECTINFO ConnectionRequest;
        struct
        {
            PCSR_CAPTURE_HEADER CaptureBuffer;
            CSR_API_NUMBER ApiNumber;
            ULONG ReturnValue;
            ULONG Reserved;
            union
            {
                CSR_CLIENTCONNECT_MSG ClientConnect;
                ULONG_PTR ApiMessageData[39];
            } u;
        };
    };
} CSR_API_MSG, *PCSR_API_MSG;

#define ROUND_UP(Count,Pow2) \
    ( ((Count)+(Pow2)-1) & (~((Pow2)-1)) )

#endif /*___5fdbd7d5_bf57_413b_b33b_35688479e703___*/
