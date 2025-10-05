	title	sioutil - utility procedures for serial I/O routines in com.c
	name	sioutil

; This module contains the following functions:
;	siosetvec	set interrupt vectors to call sioint()
;	input		input from a port
;	output		output to a port
; Note: these functions are compatible with Lattice C or Datalight C,
; small model.

_TEXT	segment byte public 'CODE'
_TEXT	ends

_DATA	segment word public 'DATA'
_DATA	ends

DGROUP	group	_DATA
	assume	cs:_TEXT,ds:DGROUP

_TEXT	segment byte public 'CODE'

	extrn	_sioint:near

; void siosetvec();

; This procedure sets the serial I/O interrupt vectors to both point to the
; INTN routine below, which in turn calls the SIOINT routine in com.c.
; The parameter to the function is the interrupt number: 4 for COM1,
; and 3 for COM2.

	public	_siosetvec
_siosetvec	proc	near
	push	bp
	mov	bp,sp
	xor	ax,ax
	mov	es,ax			; point es to base page
	mov	bx,4[bp]		; get interrupt number
	add	bx,8			; add 8 (IRQ0 is int 8)
	shl	bx,1			; multiply by 4 to get
	shl	bx,1			;  vector index
	cli				; disable interrupts
	mov	es:word ptr [bx],offset _TEXT:intn   ; set offset portion of vector
	mov	es:[bx+2],cs		; set segment portion of vector
	sti				; reenable interrupts
	pop	bp
	ret				; return to caller
_siosetvec	endp

; void intn();
;	This function is the prologue to SIOINT, which is written in C.

intn	proc	far
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
	call	_sioint			; call C interrupt procedure
	pop	di
	pop	si
	pop	dx
	pop	cx
	pop	bx
	pop	ax
	pop	ds
	pop	es
	iret
intn	endp

; input (port)
; int port;
;
; return byte from input port

	public	_input
_input	proc	near
	push	bp
	mov	bp,sp
	mov	dx,4[bp]		; get port number
	in	al,dx			; get the byte
	xor	ah,ah
	pop	bp			; return to caller
	ret
_input	endp


; output(port,data)
; int port;
; char data;
;
; output a byte to an output port

	public	_output
_output	proc	near
	push	bp
	mov	bp,sp
	mov	dx,4[bp]		; port number
	mov	al,6[bp]		; data byte to output
	out	dx,al			; send the byte to output port
	pop	bp			; return to caller
	ret
_output	endp


; enable() - enable interrupts

	public	_enable
_enable	proc	near
	sti
	ret
_enable	endp


; disable() - disable interrupts

	public	_disable
_disable	proc	near
	cli
	ret
_disable endp

_TEXT	ends

	end
