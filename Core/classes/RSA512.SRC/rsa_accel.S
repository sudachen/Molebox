
.text

.global __lv_sshl1
.equ    _A,8
.equ    _Al,12
.equ    _R,16
.equ    _Rl,20
__lv_sshl1:
        push    %ebp
        mov     %esp,%ebp
        push    %esi
        push    %edi
        push    %ecx

        movl    _A(%ebp),%esi
        movl    %esi,%edi
        addl    _Al(%ebp),%edi
        xor     %eax,%eax
        xor     %ecx,%ecx

@sshl@1:
        movzx   %al,%ecx
        shll    (%esi)
        setc    %al
        or      %ecx,(%esi)
        cmp     %esi,%edi
        je      @sshl@2
        add     $4,%esi
        jmp     @sshl@1

@sshl@2:
        movl    _R(%ebp),%esi
        movl    %esi,%edi
        addl    _Rl(%ebp),%edi

@sshl@3:
        movzx   %al,%ecx
        shll    (%esi)
        setc    %al
        or      %ecx,(%esi)
        cmp     %esi,%edi
        je      @sshl@4
        add     $4,%esi
        jmp     @sshl@3

@sshl@4:
        pop     %ecx
        pop     %edi
        pop     %esi
        pop     %ebp
        ret

.global __lv_maxbit
.equ    _A,8
.equ    _Al,12
__lv_maxbit:
        push    %ebp
        mov     %esp,%ebp
        push    %ecx
        push    %esi

        xor     %ecx,%ecx
        xor     %eax,%eax
        movl    _A(%ebp),%esi

@maxbit@1:
        bsrl    (%esi,%ecx),%edx
        je      @maxbit@2
        mov     %edx,%eax
        mov     %ecx,%edx
        shl     $3,%edx
        add     %edx,%eax
@maxbit@2:
        add     $4,%ecx
        cmp     %ecx,_Al(%ebp)
        jne     @maxbit@1

        pop     %esi
        pop     %ecx
        pop     %ebp
        ret

.equ    _A,8
.equ    _B,12
.equ    _R,16
.equ    _Bytes,20
.equ    _I,-4

.global __lv_mul0
__lv_mul0:
        push    %ebp
        mov     %esp,%ebp
        pushl   $0
        push    %esi
        push    %edi
        push    %ecx
        push    %ebx

        xor     %eax,%eax               /* memset(r,0,bytes*2) */
        movl    _R(%ebp),%edi
        mov     _Bytes(%ebp),%ecx
        shr     $1,%ecx                 /* bytes * 2 / 4 */
        rep     stosl

        movl    _A(%ebp),%esi
        movl    _B(%ebp),%ebx
        movl    _R(%ebp),%edi
        movl    $0,_I(%ebp)

@mul@1:
        xor     %ecx,%ecx

@mul@2:
        movl    (%esi),%eax             /* a[i] */
        mull    (%ebx,%ecx)             /* a[i]*b[j] -> eax,edx */
        add     %eax,(%edi,%ecx)        /* r[j] + eax */
        adc     %edx,4(%edi,%ecx)       /* r[j+1] + edx + cf */
        jc      @mul@cf                 /* while cf -> ... */
@mul@nocf:
        add     $4,%ecx                 /* ++j */
        cmp     %ecx,_Bytes(%ebp)       /* break if j == RSA_NUMBER_BYTES */
        jne     @mul@2
        movl    _I(%ebp),%eax           /* ++i */
        add     $4,%eax
        cmp     %eax,_Bytes(%ebp)       /* break if i == RSA_NUMBER_BYTES */
        je      @mul@E
        movl    %eax,_I(%ebp)
        add     $4,%esi
        add     $4,%edi
        jmp     @mul@1
@mul@E:
        pop     %ebx
        pop     %ecx
        pop     %edi
        pop     %esi
        pop     %eax
        pop     %ebp
        ret

@mul@cf:
        mov     $1,%eax
        mov     %ecx,%edx
@mul@cf@1:
        add     $4,%edx
        add     %eax,4(%edi,%edx)
        jc      @mul@cf@1
        jmp     @mul@nocf

.equ    _A,8
.equ    _R,12
.equ    _Bytes,16
.equ    _I,-4

.global __lv_add4
__lv_add4:
        push    %ebp
        mov     %esp,%ebp
        push    %esi
        push    %edi

        xor     %eax,%eax

        mov     _A(%ebp),%edx
        mov     _R(%ebp),%edi
        mov     _R(%ebp),%esi
        add     _Bytes(%ebp),%edi
        add     %edx,(%esi)
@add4@1:
        jnc     @add4@ret
        setc    %al
        add     $4,%esi
        cmp     %esi,%edi
        je      @add4@ret
        add     %eax,(%esi)
        jmp     @add4@1
@add4@ret:
        pop     %edi
        pop     %esi
        pop     %ebp
        ret


.equ    _A,8
.equ    _B,12
.equ    _Bl,16
.equ    _R,20
.equ    _Rl,24

.global __lv_mul4
__lv_mul4:
        push    %ebp
        mov     %esp,%ebp
        push    %esi
        push    %edi
        push    %ecx

        mov     _B(%ebp),%esi
        mov     %esi,%edi
        add     _Bl(%ebp),%edi

@mul4@0:
        mov     (%esi),%eax
        or      %eax,%eax
        jz      @mul4@0@2
        mull    _A(%ebp)
        pushl   _Rl(%ebp)
        pushl   _R(%ebp)
        pushl   %eax
        movl    %edx,%ecx
        call    __lv_add4
        movl    %ecx,(%esp)
        addl    $4,4(%esp)
        subl    $4,8(%esp)
        je      @mul4@0@1
        call    __lv_add4
@mul4@0@1:
        add     $12,%esp
@mul4@0@2:
        add     $4,%esi
        cmp     %esi,%edi
        je      @mul4@ret
        addl    $4,_R(%ebp)
        subl    $4,_Rl(%ebp)
        je      @mul4@ret
        jmp     @mul4@0

@mul4@ret:
        pop     %ecx
        pop     %edi
        pop     %esi
        pop     %ebp
        ret

.global __lv_less0
.equ    _A,8
.equ    _B,12
.equ    _L,16
__lv_less0:
        push    %ebp
        mov     %esp,%ebp
        push    %esi
        push    %edi
        push    %ecx

        movl    _A(%ebp),%esi
        movl    _B(%ebp),%edi
        mov     _L(%ebp),%ecx
        shr     $2,%ecx
@less@1:
        mov     -4(%esi,%ecx,4),%eax
        cmp     -4(%edi,%ecx,4),%eax
        loope   @less@1

        mov     $0,%eax
        setc    %cl
        mov     %cl,%al

        pop     %ecx
        pop     %edi
        pop     %esi
        pop     %ebp
        ret

.global __lv_sub
.equ    _A,8
.equ    _Al,12
.equ    _R,16
.equ    _Rl,20
__lv_sub:
        push    %ebp
        mov     %esp,%ebp
        push    %esi
        push    %edi
        push    %ecx

        movl    _A(%ebp),%esi
        movl    _R(%ebp),%edi
        xor     %ecx,%ecx
        xor     %edx,%edx
@sub@1:
        movl    (%esi,%ecx),%eax
        sub     %edx,(%edi,%ecx)
        setc    %dl
        sub     %eax,(%edi,%ecx)
        setc    %al
        or      %al,%dl
        add     $4,%ecx
        cmp     %ecx,_Al(%ebp)
        jne     @sub@1

@sub@4:
        or      %edx,%edx
        je      @sub@ret
        cmp     %ecx,_Rl(%ebp)
        je      @sub@ret
        sub     %edx,(%edi,%ecx)
        setc    %dl
        add     $4,%ecx
        jmp     @sub@4

@sub@ret:
        mov     %edx,%eax

        pop     %ecx
        pop     %edi
        pop     %esi
        pop     %ebp
        ret
