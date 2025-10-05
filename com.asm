	title	com - COM1 I/O for small model Turbo/Borland C
	page	66,128
	name	movb

; Defines for 8250 ports

STATUSPORT	equ	5		; line status register
DATAREADY	equ	001h		; Data Ready bit of status 
THEMPTY		equ	020h		; Transmitter ready 

; Data group

DGROUP	group	_DATA
	assume	ds:dgroup

_DATA	segment word public 'DATA'
	public	comport
comport	dw	03f8h			; default COM port
_DATA	ends

; Program group

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT

; comin() - get character from COM1

	public	_comin
_comin	proc	near
	mov	dx,comport		; get base address
	add	dx,STATUSPORT		; DX = status port
comin1:
	in	al,dx			; get status
	test	al,DATAREADY		; is data available?
	jz	comin1			; no - keep waiting
	sub	dx,STATUSPORT		; yes - look at data port
	in	al,dx			; get incoming character
	xor	ah,ah			; zero high byte
	ret
_comin	endp

; comout(c) - output c to COM1

	public	_comout
_comout	proc	near
	push	bp
	mov	bp,sp
	mov	dx,comport		; get base address
	add	dx,STATUSPORT		; DX = status port
comout1:
	in	al,dx			; get status
	test	al,THEMPTY		; is transmitter ready?
	jz	comout1			; no - keep waiting
	sub	dx,STATUSPORT		; yes - get address of data port
	mov	al,[bp+4]		; get the value to output
	out	dx,al			; send it to COM1
	pop	bp
	ret
_comout	endp

; comstat() - get receive status of COM1
; return 1 if character available, otherwise 0

	public	_comstat
_comstat proc	near
	mov	dx,comport		; get base address
	add	dx,STATUSPORT		; DX = status port
	in	al,dx			; get status
	and	al,DATAREADY		; is data available?
	jz	comstat1		; no - return 0
	mov	al,1			; yes - return 1
comstat1:
	xor	ah,ah			; zero high byte
	ret
_comstat endp

_TEXT	ends

	end
