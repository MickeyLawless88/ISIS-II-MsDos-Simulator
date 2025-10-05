	title	run80 - NEC V20 8080 emulator with ISIS-II front end
	page	66,128
	name	run80

_TEXT	segment byte public 'CODE'
_TEXT	ends

DGROUP	group	_DATA,_STACK
	assume	cs:_TEXT,ds:DGROUP

; Define the vector number and offset of the vector we shall use
; for the BRKEM (enter 8080 mode) instruction.

brkint	equ	60H		; use int 60
brkoff	equ	180h		; offset of int 60 vector

; The stack segment, allocated in START.ASM

_STACK	segment word public 'DATA'
	extrn	_stak:word	; stack for use by ISIS front end
_STACK	ends

; The data segment

_DATA	segment word public 'DATA'

	public	_savepsw,_savebc,_savede,_savehl,_savesp,_savepc,_v20

; Register values passed to ISIS-II from simulator

_savepsw dw	?		; contents of flags and A register
_savebc	dw	?		; contents of B & C registers
_savede	dw	?		; contents of D & E registers
_savehl	dw	?		; contents of H & L registers
_savesp	dw	?		; contents of stack pointer
_savepc	dw	?		; contents of program counter
_v20	db	1		; NEC V20 CPU flag

_DATA	ends

; The code segment

_TEXT	segment byte public 'CODE'

	extrn	_isis:near

; GO8080 - goto 8080 program
;
; This routine is called to emulate 8080 instructions.	It is assumed
; that locations 38H to 39H in the data space (8080 code space) contain
; a RETEM instruction (ED, FD) to return from 8080 mode.
; This RETEM is executed by the program having executed a RST 7.

; GO8080 can be used by external programs to resume emulation using the
; saved register values.

	public	_go8080

halt:

_go8080	proc	near
	mov	ax,0
	mov	es,ax			; point to interrupt page
	mov	di,brkoff		; point to BRKEM int vector
	mov	ax,_savepc		; get 8080 program counter
	cld
	stosw				; store in low word of vector
	mov	ax,ds			; get data segment
	stosw				; store in high word of vector
	mov	ax,_savepsw		; load 8080 A and flag registers
	mov	cx,_savebc		; load 8080 B,C registers
	mov	dx,_savede		; load 8080 D,E registers
	mov	bx,_savehl		; load 8080 H,L registers
	push	bp			; save our BP
	mov	bp,_savesp		; load 8080 SP register

	db	0fh,0ffh,brkint 	; enter 8080 mode (BRKEM nn)

; Assume 8080 program returns here by a RETEM instruction at 38H,
; which is executed by a RST 7 instruction. Pop the return address
; placed on the stack by the RST 7, and saves that in '_savepc'.

	mov	si,bp			; save 8080 SP register in si
	pop	bp			; restore BP from V20 stack
	mov	_savepsw,ax		; save 8080 A and flag registers
	mov	_savebc,cx		; save 8080 B,C registers
	mov	_savede,dx		; save 8080 D,E registers
	mov	_savehl,bx		; save 8080 H,L registers
	cld
	lodsw				; pop return address from RST 7
	mov	_savepc,ax		; save 8080 PC register
	mov	_savesp,si		; save 8080 SP register
	mov	sp,offset dgroup:_stak	; reset our stack
	call	_isis			; call ISIS to handle ISIS-II calls
	jmp	_go8080			; resume 8080 emulation
_go8080	endp

_TEXT	ends

	end
