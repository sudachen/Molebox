
/*

  Copyright (c) 2005-2008, Alexey Sudachen.

*/

#ifndef ___6ea2ca71_8101_4254_a1a9_32e5e7774874___
#define ___6ea2ca71_8101_4254_a1a9_32e5e7774874___

#include "./_classes.h"

using teggo::FIXEDSIZE;
using teggo::RESIZABLE;
using teggo::REUSE;
using teggo::SWAP_CONTENT;
using teggo::Uncopyable;
using teggo::NonPodCopyStrategy;
using teggo::PodCopyStrategy;
using teggo::Qsort;
using teggo::Isort;
using teggo::Shash;

using teggo::mem_ptr;
using teggo::mem_arr;
using teggo::MemPtr;
using teggo::MemArr;
using teggo::rcc_ptr;
using teggo::RccPtr;
using teggo::RccRefe;
using teggo::Refe;
using teggo::Unrefe;
using teggo::Refcounted;
using teggo::RefcountedT;
using teggo::Ireferred;
using teggo::InternList;
using teggo::WeakLink;
using teggo::WeakLinkHolder;

using teggo::GenGUIDA;
using teggo::GenGUIDW;

using teggo::LOWERTRIMMED;
using teggo::LOWERCOPY;
using teggo::TRIMMED;
using teggo::CONCATSTRINGS;
using teggo::BaseStringT;
using teggo::StringT;
using teggo::StringW;
using teggo::StringA;
using teggo::widestring;
using teggo::asciistring;
using teggo::SplitterT;
using teggo::widesplitter;
using teggo::asciisplitter;
using teggo::FormatT;
using teggo::StringParam;
using teggo::StrSafeEqual;
using teggo::StrSafeEqualI;
using teggo::StrSafeLess;
using teggo::StrSafeLessI;
using teggo::StrCompare;
using teggo::StrCompareI;
using teggo::StrLen;
using teggo::StrStartWith;
using teggo::StrStartWithI;
using teggo::StrEndWith;
using teggo::StrEndWithI;
using teggo::StrFindChar;
using teggo::StrFindRChar;
using teggo::StrSelect;
using teggo::ChrLower;
using teggo::ChrUpper;
using teggo::ChrIsAlpha;
using teggo::ChrIsAlnum;
using teggo::ChrIsDigit;
using teggo::ChrIsXdigit;
using teggo::ChrIsUpper;
using teggo::ChrIsLower;
using teggo::ToBool;
using teggo::ToLong;
using teggo::ToFloat;
using teggo::StrAsLong;
using teggo::StrAsDouble;
using teggo::Utf8String;
using teggo::WideStringFromUtf8;

using teggo::DateTime;

#if CXX_USES_EXCEPTIONS
using teggo::WideException;
#endif

using teggo::FlatmapT;
using teggo::FlatmapIteratorT;
using teggo::FlatsetT;
using teggo::FlatsetIteratorT;
using teggo::SlistT;
using teggo::KeyListT;
using teggo::ArrayT;
using teggo::BufferT;
using teggo::StackT;
using teggo::CollectionT;
using teggo::CollectionIteratorT;
using teggo::Hinstance;
using teggo::SprintF;
using teggo::VsprintF;
using teggo::Itoa;
using teggo::Itow;
using teggo::Ftoa;
using teggo::Ftow;

using teggo::GetModuleVersion;
using teggo::SysFormatError;
using teggo::ProcessYield;
using teggo::GetMillis;
using teggo::ResetMillis;
using teggo::SysMmapFile;
using teggo::SysMmapFileByHandle;
using teggo::SysUnmmapFile;
using teggo::OemRecode;
using teggo::SysGetCurrentDirectory;
using teggo::SysSetCurrentDirectory;
using teggo::SysUrandom;
using teggo::SysExec;
using teggo::SysPipeKill;
using teggo::SysPipeExitCode;
using teggo::SysPipe;
using teggo::SysPipeClose;

using teggo::IDispatchPtr;
using teggo::IUnknownPtr;
using teggo::IStreamPtr;
using teggo::ISeqStreamPtr;
using teggo::DispResult;
using teggo::DispInvoker;

using teggo::OpenLog;
using teggo::CloseLog;
using teggo::LogIsOpen;
using teggo::LogLevelGeq;

using teggo::DataSource;
using teggo::DataStream;
using teggo::NullDataStream;
using teggo::LogDataStream;
using teggo::FileDataStream;
using teggo::MemDataStream;
using teggo::ArrayDataStream;
using teggo::HandleDataStream;
using teggo::DataSourceFilter;
using teggo::DataStreamPtr;

using teggo::OpenFile; // compatibility
using teggo::OpenStream;
using teggo::CreateDataStreamFromFILE;
using teggo::OpenHandleStream;
using teggo::FileExists;
using teggo::LengthOfFile;
using teggo::CtimeOfFile;
using teggo::MtimeOfFile;
using teggo::OpenHandleStreamRtry;
using teggo::OpenStreamRtry;

using teggo::SymbolTable;

using teggo::Xnode;
using teggo::XnodeObserver;
using teggo::Xparam;
using teggo::Xiterator;
using teggo::Xdocument;
using teggo::XopenXmlDocument;
using teggo::XopenDefDocument;
using teggo::Xdocument_New;

using teggo::XMLdocumentBuilder;
using teggo::XdocumentBuilder;
using teggo::SAXparse;

using teggo::rect_t;
using teggo::frect_t;
using teggo::point_t;
using teggo::fpoint_t;
using teggo::size2_t;
using teggo::fsize2_t;

using teggo::Crc8;
using teggo::Crc16;
using teggo::Crc32;
using teggo::Adler32;
using teggo::NEWDES_Cipher;
using teggo::BLOWFISH_Cipher;
using teggo::MD5_Hash;
using teggo::Md5sign;

//using teggo::LZ_ENCODE_MEMORY;
//using teggo::LZ_DECODE_MEMORY;
//using teggo::Lz_Encode;
//using teggo::Lz_Decode;

using teggo::Tlocker;
using teggo::TsafeHolder;

using teggo::Cargs;
using teggo::CargsA;
using teggo::CargsW;
using teggo::CargsPtrA;
using teggo::CargsPtrW;
using teggo::ParseCargs;
using teggo::ParseCmdL;
using teggo::xout;
using teggo::xoutln;
using teggo::xerr;
using teggo::xerrln;
using teggo::xout8;
using teggo::xout8ln;
using teggo::xerr8;
using teggo::xerr8ln;
using teggo::EhPrintOut;
using teggo::Ehxoutln;
using teggo::ErrorFilter;
using teggo::EhFilter;
using teggo::EmptyEf;
using teggo::PRINTEF;
using teggo::EMPTYEF;
using teggo::StringEfA;
using teggo::StringEfW;
using teggo::SbufOutEfA;
using teggo::SbufOutEfW;

using teggo::DS_NOEXISTS;
using teggo::DS_REGULAR;
using teggo::DS_DIRECTORY;

using teggo::Tuple2;
using teggo::Tuple3;
using teggo::Tuple4;
using teggo::tuple2;
using teggo::tuple3;
using teggo::tuple4;

using teggo::x86copInfo;
using teggo::x86Length;

using teggo::GetBasenameOfPath;
using teggo::GetDirectoryOfPath;
using teggo::SplitPath;
using teggo::JoinPath;
using teggo::AtAppFolder;
using teggo::AtUserAppFolder;
using teggo::AtLocalUserAppFolder;
using teggo::AtCurrentDir;
using teggo::IsFullPath;

using teggo::GetCPUInfo;
using teggo::CPUINFO;
using teggo::FormatCPUInfo;

using teggo::fuslog;

#endif /*___6ea2ca71_8101_4254_a1a9_32e5e7774874___*/
