
/*

  Copyright (c) 2005-2009, Alexey Sudachen, alexey@sudachen.name

*/


#ifndef ___8860d17f_94ec_4428_a4b4_eaba0af290b5___
#define ___8860d17f_94ec_4428_a4b4_eaba0af290b5___

#include "_specific.h"

namespace teggo
{

    struct x86copInfo
    {
        int rOffs_offset;
        int rOffs_size;
        int rOffs_rel;
        int rOffs_seg;
        int op_length;
        byte_t const* op;
        int is_rjc   :1;
        int is_rcall :1;
        int is_rjmp  :1;
    };

    _TEGGO_EXPORTABLE int x86Length(byte_t const* op,x86copInfo* _info);

    #if defined _TEGGOINLINE || defined _TEGGO_X86CODE_HERE

    #if defined _TEGGOINLINE
    CXX_FAKE_INLINE
    #else
    _TEGGO_EXPORTABLE
    #endif

    int x86Length(byte_t const* op,x86copInfo* _info)
    {

        x86copInfo info;
        memset(&info,0,sizeof(info));

        enum
        {
            HAS_MODRM   = 0x010,
            JMP_REL1    = 0x020,
            JMP_REL4    = 0x040,
            JMP_ADDR6   = 0x080,
            // -1 invalid
            // -2 float operation
            LOCK_PREFIX = -5,
            REP_PREFIX  = -4,
            _0F_PREFIX  = -6,
            SEG_PREFIX  = -7,
            O16_PREFIX  = -8,
            A16_PREFIX  = -9,
        };

        static const ic8_t opcodes_info[256] =
        {
            // 00
            /*ADD*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM, 1, 4,
            /*PUSH/POP ES*/ 0,0,

            // 08
            /*OR*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM, 1, 4,
            /*PUSH CS*/ 0,
            _0F_PREFIX,

            // 10
            /*ADC*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM, 1, 4,
            /*PUSH/POP SS*/ 0, 0,

            // 18
            /*SBB*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM, 1, 4,
            /*PUSH/POP DS*/ 0, 0,

            // 20
            /*AND*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM, 1, 4,
            SEG_PREFIX /*ES*/,
            /*DAA*/ 0,

            // 28
            /*SUB*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM, 1, 4,
            SEG_PREFIX /*CS*/,
            /*DAS*/ 0,

            // 30
            /*XOR*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM, 1, 4,
            SEG_PREFIX /*SS*/,
            /*AAA*/ 0,

            // 38
            /*CMP*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM, 1, 4,
            SEG_PREFIX /*DS*/,
            /*AAS*/ 0,

            // 40
            /*INC*/  0,0,0,0,0,0,0,0,
            /*DEC*/  0,0,0,0,0,0,0,0,
            /*PUSH*/ 0,0,0,0,0,0,0,0,
            /*POP*/  0,0,0,0,0,0,0,0,

            // 60
            /*PUSHA/POPA*/ 0,0,
            /*BOUND*/ 8|HAS_MODRM,
            /*ARPL*/  HAS_MODRM,
            SEG_PREFIX /*FS*/,
            SEG_PREFIX /*GS*/,
            O16_PREFIX, /*x16 oprerand*/
            A16_PREFIX, /*x16 address*/

            // 68
            /*PUSH*/ 4,
            /*IMUL*/ 4|HAS_MODRM,
            /*PUSH*/ 1,
            /*IMUL*/ 1|HAS_MODRM,
            /*INSB*/ 0,
            /*INS*/  0,
            /*OUTSB*/0,
            /*OUTS*/ 0,

            // 70
            /*Jx*/   JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1,
            /*Jx*/   JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1, JMP_REL1,

            // 80
            /*ADD,OR,ADC,SBB,AND,SUB,XOR,CMP*/ 1|HAS_MODRM,
            /*ADD,OR,ADC,SBB,AND,SUB,XOR,CMP*/ 4|HAS_MODRM,
            /*ADD,OR,ADC,SBB,AND,SUB,XOR,CMP*/ 1|HAS_MODRM,
            /*ADD,OR,ADC,SBB,AND,SUB,XOR,CMP*/ 1|HAS_MODRM,
            /*TEST*/ HAS_MODRM,HAS_MODRM,
            /*XCHG*/ HAS_MODRM,HAS_MODRM,

            // 88
            /*MOV*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,
            /*LEA*/ HAS_MODRM,
            /*MOV*/ HAS_MODRM,
            /*POP*/ HAS_MODRM,

            // 90
            0,
            /*XCHG*/ 0,0,0,0,0,0,0,

            // 98
            /*CBW,CWD*/ 0,0,
            /*CALL*/ 6,
            /*WAIT,PUSHF,POPF,SAHF,LAHF*/ 0,0,0,0,0,

            // A0
            /*MOV*/   4,4,4,4,
            /*MOVS(B/D)*/ 0,0,
            /*CMPS(B/D)*/ 0,0,

            // A8
            /*TEST*/  1, 4,
            /*STOSB*/ 0, 0,
            /*LODSB*/ 0, 0,
            /*SCASB*/ 0, 0,

            // B0
            /*MOV*/ 1,1,1,1, 1,1,1,1,
            // B8
            /*MOV*/ 4,4,4,4, 4,4,4,4,

            // C0
            /*ROL,ROR,RCL,SHL,SHR,SAR*/ 1|HAS_MODRM, 1|HAS_MODRM,
            /*RET*/ 2, 0,
            /*LDS,LES*/ HAS_MODRM,HAS_MODRM,
            /*MOV*/ 1|HAS_MODRM, 4|HAS_MODRM,

            // C8
            /*ENTER/LEAVE*/ 2+1, 0,
            /*RETFAR*/ 2, 0,
            /*INT3/INT/INT0/IRET*/ 0,1,0,0,

            // D0
            /*ROL,ROR,RCL,SHL,SHR,SAR*/ HAS_MODRM, HAS_MODRM, HAS_MODRM, HAS_MODRM,
            /*AAM,AAD*/ 0,0,
            -1,
            /*XLAT*/ 0,

            // D8
            -2,-2,-2,-2,-2,-2,-2,-2, /*FLOAT*/

            // E0
            /*LOOP/N/E*/ JMP_REL1,JMP_REL1,JMP_REL1,
            /*JCXZ*/ JMP_REL1,
            /*IN,OUT*/ 1,4,1,4,

            // E8
            /*CALL*/ JMP_REL4,
            /*JMP*/  JMP_REL4, JMP_ADDR6, JMP_REL1,
            /*IN,OUT*/ 0,0,0,0,

            // F0
            LOCK_PREFIX,
            -1,
            REP_PREFIX,REP_PREFIX,
            0,0,
            HAS_MODRM|1,HAS_MODRM|4,

            // F8
            0,0,0,0,0,0,
            HAS_MODRM,HAS_MODRM,
        };

        static const i16_t opcodes_0f[256] =
        {
            // 0F 00
            /*SLDT,STR,LLDT,LTR,VERR,VERW*/ HAS_MODRM,
            /*SGDT, SIDT, LGDT, LIDT, SMSW,LMSW*/ HAS_MODRM,
            /*LAR,LSL*/ HAS_MODRM,HAS_MODRM,
            -1,-1,
            /*CLTS*/ 0,
            -1,

            // 0F 08
            -1, 0 -1, 0, -1, -1, -1, -1,

            // 0F 10
            -1,-1,-1,-1,-1,-1,-1,-1,

            // 0F 18
            -1,-1,-1,-1,-1,-1,-1,-1,

            // 0F 20
            /*MOV*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,
            -1,-1,-1,-1,
            // 0F 28
            -1,-1,-1,-1,-1,-1,-1,-1,

            // 0F 30
            /*WRMSR,RDTSC,RDMSR,RDPMC*/ 0,0,0,0,
            -1,-1,-1,-1,

            // 0F 38
            -1,-1,-1,-1,-1,-1,-1,-1,

            // 0F 40
            /*CMOVxx*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,

            // 0F 48
            /*CMOVxx*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,

            // 0F 50
            -1,-1,-1,-1,-1,-1,-1,-1,

            // 0F 58
            -1,-1,-1,-1,-1,-1,-1,-1,

            // 0F 60
            /*xMMX*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,

            // 0F 68
            /*xMMX*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,
            -1,-1,
            /*MOVD/Q*/ HAS_MODRM,HAS_MODRM,

            // 0F 70
            -1,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,
            /*_EMMS*/ 0,

            // 0F 78
            -1,-1,-1,-1,-1,-1,
            /*MOVD/Q*/ HAS_MODRM,HAS_MODRM,

            // 0F 80
            /*Jxx*/ JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,

            // 0F 88
            /*Jxx*/ JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,JMP_REL4,

            // 0F 90
            /*SETxx*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,

            // 0F 98
            /*SETxx*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,

            // 0F A0
            /*PUSH/POP/CPUID*/ 0,0,0,
            /*BT*/ HAS_MODRM,
            /*SHLD*/ 1|HAS_MODRM,
            /*SHLD*/ HAS_MODRM,
            -1,-1,

            // 0F A8
            /*PUSH/POP/RSM*/ 0,0,0,
            /*BTS*/ HAS_MODRM,
            /*SHRD*/ 1|HAS_MODRM,
            /*SHRD*/ HAS_MODRM,
            -1,
            /*IMUL*/ HAS_MODRM,

            // 0F B0
            /*CMPXCHG*/ HAS_MODRM,HAS_MODRM,
            /*LSS/BTR/LFS/LGS/MOVZX/MOVZX*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,

            // 0F B8
            /*Bxx*/ -1,-1,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,
            /*MOVSX*/ HAS_MODRM,HAS_MODRM,

            // 0F C0
            /*XADD*/ HAS_MODRM, HAS_MODRM,
            -1,-1,-1,-1,-1,-1,

            // 0F C8
            /*BSWAP*/ 0,0,0,0,0,0,0,0,

            // 0F D0
            -1,
            /*xMMX*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,
            -1,
            /*PMULLW*/ HAS_MODRM,
            -1,-1,

            // 0F D8
            /*xMMX*/ HAS_MODRM,HAS_MODRM,-1,HAS_MODRM,HAS_MODRM,HAS_MODRM,-1,HAS_MODRM,

            // 0F E0
            /*xMMX*/ -1,HAS_MODRM,HAS_MODRM,HAS_MODRM,-1,HAS_MODRM,-1,-1,

            // 0F E8
            /*xMMX*/ HAS_MODRM,HAS_MODRM,-1,HAS_MODRM,HAS_MODRM,HAS_MODRM,-1,HAS_MODRM,

            // 0F F0
            /*xMMX*/ -1,HAS_MODRM,HAS_MODRM,HAS_MODRM,-1,HAS_MODRM,-1,-1,

            // 0F F8
            /*xMMX*/ HAS_MODRM,HAS_MODRM,HAS_MODRM,-1,HAS_MODRM,HAS_MODRM,HAS_MODRM,HAS_MODRM,
        };

        int o_16 = 0, a_16 = 0;

        ic8_t const* table_0 = opcodes_info;
        ic8_t code_info = table_0[*op];
        //ic8_t code_info = opcodes_info[*op];
        info.op_length = 1;

    loop:
        if ( code_info < 0 )
        {
            if ( code_info >= -2 ) return 0;
            ++op;
            ++info.op_length;
            switch ( code_info )
            {
                case _0F_PREFIX:
                    code_info = opcodes_0f[*op];
                    goto loop;
                case O16_PREFIX:
                    o_16 = 1;
                    goto _1;
                case A16_PREFIX:
                    a_16 = 1;
                    goto _1;
                _1:
                default:
                    code_info = opcodes_info[*op];
                    goto loop;
            }
        }
        else
        {
            info.op = op;
            ++op; // skip opcode
            int a_size = code_info & 0x0f;
            if ( o_16 && a_size >=4 ) a_size = (a_size>>1) + a_size&3;
            info.op_length += a_size;
            if ( code_info & HAS_MODRM )
            {
                int mod = *op >> 6;
                int r_m = *op & 7;
                if ( !a_16 )
                    switch ( mod )
                    {
                        case 0:
                            if ( r_m == 4 ) ++info.op_length;           // sib
                            else if ( r_m == 5 ) info.op_length += 4;   // offs32
                            break;
                        case 1:
                            ++info.op_length;                           // offs8
                            if ( r_m == 4 ) ++info.op_length;           // sib
                            break;
                        case 2:
                            info.op_length+=4;                          // offs32
                            if ( r_m == 4 ) ++info.op_length;           // sib
                    }
                else // with ADDR16 prefix
                    switch ( mod )
                    {
                        case 0:
                            if ( r_m == 6 ) info.op_length += 2;        // offs16
                            break;
                        case 1:
                            ++info.op_length;                           // offs8
                            break;
                        case 2:
                            info.op_length+=2;                          // offs16
                            break;
                    }
                ++op;
                ++info.op_length; // modr/m
            }

            if ( (code_info & 0x0e0) && !(code_info & 0x10) )
            {
                info.is_rjc = 1;
                info.rOffs_offset = info.op_length;
                info.rOffs_size = (code_info >> 4) & 0x0f;
                if ( info.rOffs_size == 2 ) info.rOffs_size = 1;
                switch ( info.rOffs_size )
                {
                    case 1: info.rOffs_rel = *(ic8_t*)op; break;
                    case 4: info.rOffs_rel = *(i32_t*)op; break;
                    case 6:
                        info.rOffs_seg = *(u16_t*)op;
                        info.rOffs_rel = *(i32_t*)(op+2);
                        break;
                }
                if ( *info.op == 0xe8 ) info.is_rcall = 1;
                if ( *info.op == 0xe9 || *info.op == 0xea || *info.op == 0xeb  ) info.is_rjmp = 1;
                info.op_length += info.rOffs_size;
            }
        }

        if ( _info ) *_info = info;
        return info.op_length;
    }

    #endif

}

#endif /*___8860d17f_94ec_4428_a4b4_eaba0af290b5___*/
