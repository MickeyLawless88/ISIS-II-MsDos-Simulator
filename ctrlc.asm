	title	ctrlc - routines for catching control-C or control-Break
	name	sioutil

; This module contains the following function:
;	ctrlc	catch control-C, call C routine
; Note: this function is compatible with Lattice C or Datalight C,
; small model.

_TEXT	segment byte public 'CODE'
_TEXT	ends

DGROUP	group	_DATA
	assume	cs:_TEXT,ds:DGROUP

_DATA	segment word public 'DATA'
codeadd	dw	?			; address of c routine to call
_DATA	ends

_TEXT	segment byte public 'CODE'

; void ctrlc(code);
; void (*code)();

; This procedure sets the Control-Break vector (0x23) to point to
; INTN routine below, which in turn calls the user-specified C routine.
; The C routine's address is passed as the parameter to this function.

	public	_ctrlc
_ctrlc	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
	push	ds			; save ds for DOS call
	mov	ax,4[bp]		; get user's code address
	mov	codeadd,ax		; save for later
	mov	ax,cs			; must copy cs to ds
	mov	ds,ax
	mov	dx,offset _TEXT:cint	; ds:dx -> cint()
	mov	ah,25h			; DOS call 25H to set
	mov	al,23h			;  vector for int 23H, which is
	int	21h			;  control-C vector
	pop	ds			; restore ds
	pop	di
	pop	si
	pop	bp
	ret				; return to caller
_ctrlc	endp

; void cint();
;	This function is given control when Control-C is hit.
;	It saves registers, then calls the user's C routine.

cint	proc	far
	push	es
	push	ds
	push	ax
	push	bx
	push	cx
	push	dx
	push	si
	push	di
	mov	ax,dgroup
	mov	ds,ax			; load small model DS value
	call	codeadd			; call C interrupt procedure
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	es
	iret
cint	endp

_TEXT	ends
	end
