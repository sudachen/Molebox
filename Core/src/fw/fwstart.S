.section .text$start, "rx"

.global _Entry
_Entry:
        jmp	__Molebox_Install
.global _Inject
_Inject:
        jmp	__Molebox_Inject
.align 16
__Core_Offset_Value:
.long   0x4b495551
__Core_Size_Value:
.long   0x31584f42
__Core_Key:
.byte   0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5

.global __Core_Size
__Core_Size:
        call	__Core_Size@e
__Core_Size@e:
        pop     %eax
        sub     $(__Core_Size@e-__Core_Size_Value),%eax
        mov     (%eax),%eax
        ret
.global __Core_Offset
__Core_Offset:
        call	__Core_Offset@e
__Core_Offset@e:
        pop     %eax
        sub     $(__Core_Offset@e-__Core_Offset_Value),%eax
        mov     (%eax),%eax
        ret
.global __Self_Ptr
__Self_Ptr:
        call    __Self_Ptr@e
__Self_Ptr@e:
        pop     %eax
        sub     $(__Self_Ptr@e-_Entry),%eax
        ret
.global __Self_Key
__Self_Key:
        call    __Self_Key@e
__Self_Key@e:
        pop     %eax
        sub     $(__Self_Key@e-__Core_Key),%eax
        ret

__Molebox_Install:
        movl    4(%esp),%eax
        or      %eax,%eax
        jne     _Molebox_Install
        jmp     _Molebox_Init
