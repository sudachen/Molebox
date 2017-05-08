
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/

#include "_specific.h"
#include "string.h"
#include "format.h"

namespace teggo
{

    struct CPUINFO
    {
        char id[16];
        char tag[64];
        unsigned family;
        unsigned model;
        unsigned stepping;
        unsigned revision;
        unsigned number;
        unsigned MMX:1;
        unsigned MMX2:1;
        unsigned SSE:1;
        unsigned SSE2:1;
        unsigned SSE3:1;
        unsigned SSSE3:1;
        unsigned HTT:1;
        unsigned EST:1;
        unsigned PAE:1;
    };

    inline void QueryCPUID(void* rgs,int id)
    {
        __asm mov eax, id
        __asm push esi
        __asm push ebx
        __asm push ecx
        __asm push edx
        __asm _emit 0x0f
        __asm _emit 0xa2
        __asm mov esi,rgs
        __asm mov [esi],eax
        __asm mov [esi+4],ebx
        __asm mov [esi+8],ecx
        __asm mov [esi+12],edx
        __asm pop edx
        __asm pop ecx
        __asm pop ebx
        __asm pop esi
    }

    CXX_FAKE_INLINE void GetCPUInfo(CPUINFO* cpui)
    {
        unsigned r[4];
        memset(cpui,0,sizeof(*cpui));

        QueryCPUID(r,0);
        *(unsigned*)(cpui->id+0) = r[1];
        *(unsigned*)(cpui->id+4) = r[3];
        *(unsigned*)(cpui->id+8) = r[2];

        QueryCPUID(r,1);
        cpui->family = (r[0]>>8)&0x0f;
        cpui->model  = (r[0]>>4)&0x0f;
        cpui->stepping  = r[0]&0x0f;
        //if ( 2 == (r[0] & 0x3000) >> 12 ) cpui->dual = 1;
        cpui->number = (r[1] >> 16) &0xff;

        if ( r[3] & CXX_BIT(23) ) cpui->MMX = 1;
        if ( r[3] & CXX_BIT(24) ) cpui->MMX2 = 1;
        if ( r[3] & CXX_BIT(25) ) cpui->SSE = 1;
        if ( r[3] & CXX_BIT(26) ) cpui->SSE2 = 1;
        if ( r[3] & CXX_BIT(28) ) cpui->HTT = 1;
        if ( r[2] & CXX_BIT(0) )  cpui->SSE3 = 1;
        if ( r[2] & CXX_BIT(9) )  cpui->SSSE3 = 1;
        if ( r[2] & CXX_BIT(7) )  cpui->EST = 1;
        if ( r[2] & CXX_BIT(6) )  cpui->PAE = 1;


        QueryCPUID(cpui->tag,0x80000002);
        QueryCPUID(cpui->tag+16,0x80000003);
        QueryCPUID(cpui->tag+32,0x80000004);
        for ( int i = 1; i < 64 && cpui->tag[i] ; )
        {
            if ( cpui->tag[i] == cpui->tag[i-1] && cpui->tag[i] == ' ')
                memmove(cpui->tag+i-1,cpui->tag+i,64-i);
            else ++i;
        }
    }


    CXX_FAKE_INLINE StringA FormatCPUInfo(CPUINFO* cpui)
    {
        StringA fts = "";
        if ( cpui->MMX ) fts.Append("mmx");
        if ( cpui->MMX2 ) { if (fts) fts.Append(" "); fts.Append("mmx2"); }
        if ( cpui->SSE ) { if (fts) fts.Append(" "); fts.Append("sse"); }
        if ( cpui->SSE2 ) { if (fts) fts.Append(" "); fts.Append("sse2"); }
        if ( cpui->SSE3 ) { if (fts) fts.Append(" "); fts.Append("sse3"); }
        if ( cpui->SSSE3 ) { if (fts) fts.Append(" "); fts.Append("ssse3"); }
        if ( cpui->HTT ) { if (fts) fts.Append(" "); fts.Append("htt"); }
        if ( cpui->EST ) { if (fts) fts.Append(" "); fts.Append("est"); }
        if ( cpui->PAE ) { if (fts) fts.Append(" "); fts.Append("pae"); }

        return StringA(_S*"%s {%s} %s"
                       %cpui->id
                       %fts
                       %cpui->tag);
    }

} // namespace
