.text
.global _memcopy
_memcopy:
	push 	%esi
	push	%edi
	push	%ecx
	mov	16(%esp),%edi
	mov	20(%esp),%esi
	mov	24(%esp),%ecx
/*
_memcopy@4:
	cmp	%ecx,$4
	jb	_memcopy@1
	mov	(%esi),%eax
	mov	%eax,(%edi)
	sub	$4,%ecx
	add	$4,%esi
	add	$4,%edi
	jmp	_memcopy@4
_memcopy@1:
*/
	or	%ecx,%ecx
	jz	_memcpy@exit
_memcopy@1l:
	mov	(%esi),%al
	mov	%al,(%edi)
	inc	%esi
	inc	%edi
	dec	%ecx
	jnz	_memcopy@1l
_memcpy@exit:
	pop	%ecx
	pop	%edi
	pop	%esi
	ret

.global _memsetS
_memsetS:
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

.global __TIB
__TIB:
	mov 	%fs:0x18, %eax
	ret

.global __PEB
__PEB:
	call 	__TIB
	mov	48(%eax),%eax
	ret	

.global __LASTSTATUS
__LASTSTATUS:
	call 	__TIB
	add		$3060,%eax
	ret

.global __PROCPARAM
__PROCPARAM:
	call  	__PEB
	mov		16(%eax),%eax
	ret

.global __CURDIRHANDLE
__CURDIRHANDLE:
	call	__PROCPARAM
	add		$36,%eax  /* CURDIR */
 	mov		8(%eax),%eax
	ret
            
.global __Internal_Exec
__Internal_Exec:
	mov	4(%esp),%eax
	call	*%eax
	push	%eax
	call	_ExitProcess@4

.section .rdata, "r"
	ret
