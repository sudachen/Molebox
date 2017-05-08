
/*

  Copyright (c) 2008-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_common.h"

// \\HKEY_LOCAL_MACHINE\Software\Microsoft\Windows NT\CurrentVersion\AeDebug

EXTERN_C void StartDebugger()
{
    unsigned long pid = GetProcessId(INVALID_HANDLE_VALUE);
    STARTUPINFO s0 = {0};
    PROCESS_INFORMATION p0 = {0};
    CreateProcess(
        0,
        (char*)(0|_S*"windbg -p %d "%pid),
        0,
        0,
        0,
        0,
        0,
        0,
        &s0,&p0);
    Sleep(5000);
    __asm int 3
    ;
}

EXTERN_C int AuxReadFile(HANDLE f,void* ptr, unsigned len)
{
    unsigned long foo = 0;
    byte_t* p = (byte_t*)ptr;
    do
    {
        foo = 0;
        if ( ReadFile(f,p,len,&foo,0) && foo )
        { p += foo; len-=foo; }
        else
            return 0;
        //else
        //  __asm__(("int3"));
    }
    while (len);
    return 1;
}

EXTERN_C void* AuxAllocExec(int sz)
{
    void* m = 0;
    if ( m = VirtualAlloc(0,sz,MEM_COMMIT,PAGE_EXECUTE_READWRITE) )
        return m;
    if ( m = VirtualAlloc(0,sz,MEM_COMMIT,PAGE_READWRITE) )
        return m;
    __asm int 3
    ;
    return 0;
}

EXTERN_C void AuxFree(void* p)
{
    VirtualFree(p,0,MEM_RELEASE);
}


