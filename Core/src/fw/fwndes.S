.section .text$ndes, "rx"

//.global NDESTABLE
NDESTABLE:
	call NDESTABLE@e
.byte  32,137,239,188,102,125,221, 72,212, 68, 81, 37, 86,237,147,149
.byte  70,229, 17,124,115,207, 33, 20,122,143, 25,215, 51,183,138,142
.byte 146,211,110,173,  1,228,189, 14,103, 78,162, 36,253,167,116,255
.byte 158, 45,185, 50, 98,168,250,235, 54,141,195,247,240, 63,148,  2
.byte 224,169,214,180, 62, 22,117,108, 19,172,161,159,160, 47, 43,171
.byte 194,175,178, 56,196,112, 23,220, 89, 21,164,130,157,  8, 85,251
.byte 216, 44, 94,179,226, 38, 90,119, 40,202, 34,206, 35, 69,231,246
.byte  29,109, 74, 71,176,  6, 60,145, 65, 13, 77,151, 12,127, 95,199
.byte  57,101,  5,232,150,210,129, 24,181, 10,121,187, 48,193,139,252
.byte 219, 64, 88,233, 96,128, 80, 53,191,144,218, 11,106,132,155,104
.byte  91,136, 31, 42,243, 66,126,135, 30, 26, 87,186,182,154,242,123
.byte  82,166,208, 39,152,190,113,205,114,105,225, 84, 73,163, 99,111
.byte 204, 61,200,217,170, 15,198, 28,192,254,134,234,222,  7,236,248
.byte 201, 41,177,156, 92,131, 67,249,245,184,203,  9,241,  0, 27, 46
.byte 133,174, 75, 18, 93,209,100,120, 76,213, 16, 83,  4,107,140, 52
.byte  58, 55,  3,244, 97,197,238,227,118, 49, 79,230,223,165,153, 59
NDESTABLE@e:
	pop	%ebx
	ret

.global _Do_Cipher /* _cdecl (void *key, void *b, int count) */
_Do_Cipher:

        testl   $-1,12(%esp)
        je      l_ret

        push    %ebp
        mov     %esp,%ebp
        push    %esi
        push    %edi
        push    %ecx
        push    %ebx

        call    NDESTABLE
     	//	table -> &ebx
        mov     12(%ebp),%esi   /* b */

l_count:
        mov     8(%ebp),%edi    /* key */
        mov     (%esi),%edx

        push    %esi
        mov     4(%esi),%esi
        pushl   $9

        /* %edx - first word, %esi - second */
l_loop:
        /* *b+4 = *b+4 ^ rotor[ *b ^ *key ] */
        mov     %edx,%eax
        xor     (%edi),%eax
        add     $4,%edi
        movzx   %al,%ecx
        mov     (%ebx,%ecx),%al
        movzx   %ah,%ecx
        mov     (%ebx,%ecx),%ah
        ror     $16,%eax
        movzx   %al,%ecx
        mov     (%ebx,%ecx),%al
        movzx   %ah,%ecx
        mov     (%ebx,%ecx),%ah
        ror     $16,%eax
        xor     %esi,%eax

        decl    (%esp)
        jz      l_end

        mov     %eax,%esi       /* esi = word(b+4) */
        xor     %al,%ah         /* b[5] ^= b[4] */
        xor     (%edi),%eax     /* *b   ^= *k   */
        add     $4,%edi

        movzx   %al,%ecx
        mov     (%ebx,%ecx),%al
        movzx   %ah,%ecx
        mov     (%ebx,%ecx),%ah
        ror     $16,%eax
        movzx   %al,%ecx
        mov     (%ebx,%ecx),%al
        movzx   %ah,%ecx
        mov     (%ebx,%ecx),%ah
        ror     $8,%eax

        xor     %eax,%edx       /* edx = word(b) */
        jmp     l_loop

l_end:
        pop     %esi
        pop     %esi
        mov     %edx,(%esi)
        mov     %eax,4(%esi)
        add     $8,%esi
        decl    16(%ebp)        /* count */
        jnz     l_count

        pop     %ebx
        pop     %ecx
        pop     %edi
        pop     %esi
        pop     %ebp

l_ret:
        ret
