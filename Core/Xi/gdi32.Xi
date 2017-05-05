#
# splited import table declaration
# $Id$
#

AddFontResourceA,int,LPCTSTR
RemoveFontResourceA,BOOL,LPCTSTR
DeleteDC,BOOL,HDC
CreateDIBSection,HBITMAP,HDC,CONST BITMAPINFO *,UINT,VOID **,HANDLE,DWORD
CreateCompatibleDC,HDC,HDC
BitBlt,BOOL,HDC,int,int,int,int,HDC,int,int,DWORD
SelectObject,HGDIOBJ,HDC,HGDIOBJ
GetObjectA,int,HGDIOBJ,int,LPVOID
DeleteObject,BOOL,HGDIOBJ
GetDC,HDC,HWND
ReleaseDC,BOOL,HWND,HDC
SetDIBits,int,HDC,HBITMAP,UINT,UINT,VOID*,BITMAPINFO*,UINT
CreateCompatibleBitmap,HBITMAP,HDC,int,int

