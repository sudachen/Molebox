
.global __NtDlF
__NtDlF:
__DlF_HANDLER:
	mov	$0x7fffffff,%eax
__DlF_ORIGN:
	push	$0x7fffffff	
	or	%eax,%eax
	jnz	__NtDlF@E
	ret
__NtDlF@E:
__DlF_RETN:
	push	$0x7fffffff

	pushl	$0   	/* result */
	push	%esp 	/* & result */

	/*
        &result
        -------
	result     +4
	retn       +8
	orign      +12
	retaddress +16
	args       +20
	*/

	push	%esp
	add	$20,(%esp)
__DlF_APIFID:
	push	$0x7fffffff

	/*
	apifid
	&args
	&result
	------
	*/

	jmp	*%eax
	int3
__NtDlF0_END:

.global __DlF_COPY /* (dst) */
__DlF_COPY:
	mov	4(%esp),%eax
	pushl	$(__NtDlF0_END-__NtDlF)
	push	$__NtDlF
	push	%eax
	call	_memcopy
	add	$12,%esp
	ret
	
.global __DlF_SIZE
__DlF_SIZE:
	mov	$(__NtDlF0_END-__NtDlF),%eax
	ret

.global __DlF_SET_ORIGN /* (p, val) */
__DlF_SET_ORIGN:
	mov	4(%esp),%eax
	mov	8(%esp),%edx
	add	$(__DlF_ORIGN-__NtDlF+1),%eax
	mov	%edx,(%eax)
	ret

.global __DlF_SET_APIFID /* (p, val) */
__DlF_SET_APIFID:
	mov	4(%esp),%eax
	mov	8(%esp),%edx
	add	$(__DlF_APIFID-__NtDlF+1),%eax
	mov	%edx,(%eax)
	ret

.global __DlF_SET_RETN /* (p, val) */
__DlF_SET_RETN:
	mov	4(%esp),%eax
	mov	8(%esp),%edx
	add	$(__DlF_RETN-__NtDlF+1),%eax
	mov	%edx,(%eax)
	ret

.global __DlF_SET_HANDLER /* (p, val) */
__DlF_SET_HANDLER:
	mov	4(%esp),%eax
	mov	8(%esp),%edx
	add	$(__DlF_HANDLER-__NtDlF+1),%eax
	mov	%edx,(%eax)
	ret

.global __NtDlFhandler
__NtDlFhandler:

	push %esi
	push %edi
	push %ebx
	push %ecx
	call	_APIF_DoCall /* (apifid,args,&result) */
	pop  %ecx
	pop  %ebx
	pop  %edi
	pop  %esi
	add  $12,%esp
	
	/*
	result
        retn   +4
        orign  +8
        ---------
	*/

	or	%eax,%eax       	/* RETURN ? */
	jnz	__NtDlFhandler@1
	mov	12(%esp), %eax 		/* retaddress */
	mov	%esp,%edx
	add	4(%esp),%edx
	add	$12,%edx
	mov	%eax,(%edx)
	pop	%eax
	mov	%edx,%esp	
	ret

__NtDlFhandler@1: /* call to orign */
	
	add	$8,%esp
	ret
			