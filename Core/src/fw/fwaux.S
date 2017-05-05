.section .text$aux, "rx"

.global __Query_DT64
__Query_DT64:
	call __Query_DT64@e
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 0  ,1  ,2  ,3  ,4  ,5  ,6  ,7  ,8  ,9  ,255,255,255,255,255,255
.byte 63 ,36 ,37 ,38 ,39 ,40 ,41 ,42 ,43 ,44 ,45 ,46 ,47 ,48 ,49 ,50 
.byte 51 ,52 ,53 ,54 ,55 ,56 ,57 ,58 ,59 ,60 ,61 ,255,255,255,255,62 
.byte 255,10 ,11 ,12 ,13 ,14 ,15 ,16 ,17 ,18 ,19 ,20 ,21 ,22 ,23 ,24 
.byte 25 ,26 ,27 ,28 ,29 ,30 ,31 ,32 ,33 ,34 ,35 ,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
.byte 255,255,255,255,255,255,255,255,255,255,255,255,255,255,255,255
__Query_DT64@e:
	pop	%eax
	ret

CHRCMP:
	xorb	(%esi),%al
	je	CHRCMP_L
	xorb	(%esi),%ah
	je	CHRCMP_L
	xor	%eax,%eax
	ret
CHRCMP_L:
	shr	$16,%eax
	add	$2,%esi
	or	%eax,%eax
	jnz 	CHRCMP
CHRCMP_R:
	mov	$1,%eax
	ret

__ISNDLL:
	xor	%eax,%eax
	mov	16(%esp),%esi
	mov	40(%esi),%esi
__DLONS_L:
	mov	(%esi),%ax
	add 	$2,%esi
	or	%eax,%eax
	jz	__DLONS_L1
	xor	$'\\',%eax
	jnz	__DLONS_L2
	mov	%esi,%edi
__DLONS_L2:
	jmp	__DLONS_L
__DLONS_L1:
	mov	%edi,%esi
	ret

.global _ISNTDLL
_ISNTDLL:
	push	%esi
	push	%edi
	call	__ISNDLL
	mov	$(0x54744e6e),%eax /*NnTt*/
	call	CHRCMP
	or	%eax,%eax
	jz	ISNTDLL_R
	mov	$(0x4c6c4464),%eax /*DdLl*/
	call	CHRCMP
	or	%eax,%eax
	jz	ISNTDLL_R
ISNTDLL_R:
	pop	%edi
	pop	%esi
        ret

.global _ISK32DLL
_ISK32DLL:
	push 	%esi
	push 	%edi
	call	__ISNDLL
	mov	$(0x45654b6b),%eax /*KkEe*/
	call	CHRCMP
	or	%eax,%eax
	jz	ISNTDLL_R
	mov	$(0x6e4e5272),%eax /*RrNn*/
	call	CHRCMP
	or	%eax,%eax
	jz	ISNTDLL_R
	mov	$(0x4c6c4565),%eax /*EeLl*/
	call	CHRCMP
	or	%eax,%eax
	jz	ISNTDLL_R
	mov	$(0x32323333),%eax /*3322*/
	call	CHRCMP
	or	%eax,%eax
	jz	ISNTDLL_R
ISK32DLL_R:
	pop	%edi
	pop	%esi
        ret

.global __TIB
__TIB:
	mov	%fs:0x18, %eax
	ret

.global __PEB
__PEB:
	call	__TIB
	mov	48(%eax),%eax
	ret	

.global __DLO
__DLO:
	call	__PEB
	mov	12(%eax),%eax
	add	$12,%eax
	ret

.global _memset
_memset:
	push 	%edi
	push 	%ecx
	mov	12(%esp),%edi
	mov	16(%esp),%eax
	mov	20(%esp),%ecx
	or	%ecx,%ecx
	jz	_memset@e
_memset@l:
	mov	%al,(%edi)
	inc	%edi
	dec	%ecx
	jnz	_memset@l
_memset@e:
	pop	%ecx
	pop	%edi
	ret	

.global _memeq
_memeq:
	push	%esi
	push	%edi
	push	%ecx
	mov		16(%esp),%edi
	mov		20(%esp),%esi
	mov		24(%esp),%ecx
	xor 	%eax,%eax
	or		%ecx,%ecx
	jz		_memeq@exit
_memeq@1l:
	mov		(%esi),%al
	sub		(%edi),%al
	jnz 	_memeq@exit
	inc		%esi
	inc		%edi
	dec		%ecx
	jnz		_memeq@1l
_memeq@exit:
	pop		%ecx
	pop		%edi
	pop		%esi
	ret

.global __Molebox_Inject
__Molebox_Inject:
	push	$0
	jmp	_Molebox_Inject
