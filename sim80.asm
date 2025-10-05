	title	sim80 - 8080 simulator with ISIS-II front end
	page	66,128
	name	sim80

_TEXT	segment byte public 'CODE'
_TEXT	ends

DGROUP	group	_DATA,_STACK
	assume	cs:_TEXT,ds:DGROUP

_STACK	segment word public 'DATA'
	extrn	_stak:word	; stack for use by ISIS front end
_STACK	ends

_DATA	segment word public 'DATA'

	public	_savepsw,_savebc,_savede,_savehl,_savesp,_savepc,_v20

; Register values passed to ISIS-II from simulator

_savepsw dw	?		; contents of flags and A register
_savebc	dw	?		; contents of B & C registers
_savede	dw	?		; contents of D & E registers
_savehl	dw	?		; contents of H & L registers
_savesp	dw	?		; contents of stack pointer
_savepc	dw	?		; contents of program counter

; JMPTAB - table of instruction handlers, indexed by opcode 0-255

jmptab	dw	offset _TEXT:fetch,offset _TEXT:lxib	   ; 0,1
	dw	offset _TEXT:staxb,offset _TEXT:inxb	   ; 2,3
	dw	offset _TEXT:inrb,offset _TEXT:dcrb	   ; 4,5
	dw	offset _TEXT:mvib,offset _TEXT:rlc	   ; 6,7
	dw	offset _TEXT:halt, offset _TEXT:dadb	   ; 8,9
	dw	offset _TEXT:ldaxb,offset _TEXT:dcxb	   ; a,b
	dw	offset _TEXT:inrc,offset _TEXT:dcrc	   ; c,d
	dw	offset _TEXT:mvic,offset _TEXT:rrc	   ; e,f
	dw	offset _TEXT:halt, offset _TEXT:lxid	   ; 10,11
	dw	offset _TEXT:staxd,offset _TEXT:inxd	   ; 12,13
	dw	offset _TEXT:inrd,offset _TEXT:dcrd	   ; 14,15
	dw	offset _TEXT:mvid,offset _TEXT:ral	   ; 16,17
	dw	offset _TEXT:halt, offset _TEXT:dadd	   ; 18,19
	dw	offset _TEXT:ldaxd,offset _TEXT:dcxd	   ; 1a,1b
	dw	offset _TEXT:inre,offset _TEXT:dcre	   ; 1c,1d
	dw	offset _TEXT:mvie,offset _TEXT:rar	   ; 1e,1f
	dw	offset _TEXT:halt, offset _TEXT:lxih	   ; 20,21
	dw	offset _TEXT:shldx, offset _TEXT:inxh	   ; 22,23
	dw	offset _TEXT:inrh,offset _TEXT:dcrh	   ; 24,25
	dw	offset _TEXT:mvih,offset _TEXT:daa1	   ; 26,27
	dw	offset _TEXT:halt, offset _TEXT:dadh	   ; 28,29
	dw	offset _TEXT:lhld, offset _TEXT:dcxh	   ; 2a,2b
	dw	offset _TEXT:inrl,offset _TEXT:dcrl	   ; 2c,2d
	dw	offset _TEXT:mvil,offset _TEXT:cma	   ; 2e,2f
	dw	offset _TEXT:halt, offset _TEXT:lxisp    ; 30,31
	dw	offset _TEXT:sta,  offset _TEXT:inxsp    ; 32,33
	dw	offset _TEXT:inrm,offset _TEXT:dcrm	   ; 34,35
	dw	offset _TEXT:mvim,offset _TEXT:stc1	   ; 36,37
	dw	offset _TEXT:halt, offset _TEXT:dadsp    ; 38,39
	dw	offset _TEXT:lda,  offset _TEXT:dcxsp    ; 3a,3b
	dw	offset _TEXT:inra,offset _TEXT:dcra	   ; 3c,3d
	dw	offset _TEXT:mvia,offset _TEXT:cmc1	   ; 3e,3f
	dw	offset _TEXT:movbb,offset _TEXT:movbc    ; 40,41
	dw	offset _TEXT:movbd,offset _TEXT:movbe    ; 42,43
	dw	offset _TEXT:movbh,offset _TEXT:movbl    ; 44,45
	dw	offset _TEXT:movbm,offset _TEXT:movba    ; 46,47
	dw	offset _TEXT:movcb,offset _TEXT:movcc    ; 48,49
	dw	offset _TEXT:movcd,offset _TEXT:movce    ; 4a,4b
	dw	offset _TEXT:movch,offset _TEXT:movcl    ; 4c,4d
	dw	offset _TEXT:movcm,offset _TEXT:movca    ; 4e,4f
	dw	offset _TEXT:movdb,offset _TEXT:movdc    ; 50,51
	dw	offset _TEXT:movdd,offset _TEXT:movde    ; 52,53
	dw	offset _TEXT:movdh,offset _TEXT:movdl    ; 54,55
	dw	offset _TEXT:movdm,offset _TEXT:movda    ; 56,57
	dw	offset _TEXT:moveb,offset _TEXT:movec    ; 58,59
	dw	offset _TEXT:moved,offset _TEXT:movee    ; 5a,5b
	dw	offset _TEXT:moveh,offset _TEXT:movel    ; 5c,5d
	dw	offset _TEXT:movem,offset _TEXT:movea    ; 5e,5f
	dw	offset _TEXT:movhb,offset _TEXT:movhc    ; 60,61
	dw	offset _TEXT:movhd,offset _TEXT:movhe    ; 62,63
	dw	offset _TEXT:movhh,offset _TEXT:movhl    ; 64,65
	dw	offset _TEXT:movhm,offset _TEXT:movha    ; 66,67
	dw	offset _TEXT:movlb,offset _TEXT:movlc    ; 68,69
	dw	offset _TEXT:movld,offset _TEXT:movle    ; 6a,6b
	dw	offset _TEXT:movlh,offset _TEXT:movll    ; 6c,6d
	dw	offset _TEXT:movlm,offset _TEXT:movla    ; 6e,6f
	dw	offset _TEXT:movmb,offset _TEXT:movmc    ; 70,71
	dw	offset _TEXT:movmd,offset _TEXT:movme    ; 72,73
	dw	offset _TEXT:movmh,offset _TEXT:movml    ; 74,75
	dw	offset _TEXT:halt, offset _TEXT:movma    ; 76,77
	dw	offset _TEXT:movab,offset _TEXT:movac    ; 78,79
	dw	offset _TEXT:movad,offset _TEXT:movae    ; 7a,7b
	dw	offset _TEXT:movah,offset _TEXT:moval    ; 7c,7d
	dw	offset _TEXT:movam,offset _TEXT:movaa    ; 7e,7f
	dw	offset _TEXT:addb,offset _TEXT:addc	   ; 80,81
	dw	offset _TEXT:addd,offset _TEXT:adde	   ; 82,83
	dw	offset _TEXT:addh,offset _TEXT:addl	   ; 84,85
	dw	offset _TEXT:addm,offset _TEXT:adda	   ; 86,87
	dw	offset _TEXT:adcb,offset _TEXT:adcc	   ; 88,89
	dw	offset _TEXT:adcd,offset _TEXT:adce	   ; 8a,8b
	dw	offset _TEXT:adch,offset _TEXT:adcl	   ; 8c,8d
	dw	offset _TEXT:adcm,offset _TEXT:adca	   ; 8e,8f
	dw	offset _TEXT:subb,offset _TEXT:subc	   ; 90,91
	dw	offset _TEXT:subd,offset _TEXT:sube	   ; 92,93
	dw	offset _TEXT:subh,offset _TEXT:subl	   ; 94,95
	dw	offset _TEXT:subm,offset _TEXT:suba	   ; 96,97
	dw	offset _TEXT:sbbb,offset _TEXT:sbbc	   ; 98,99
	dw	offset _TEXT:sbbd,offset _TEXT:sbbe	   ; 9a,9b
	dw	offset _TEXT:sbbh,offset _TEXT:sbbl	   ; 9c,9d
	dw	offset _TEXT:sbbm,offset _TEXT:sbba	   ; 9e,9f
	dw	offset _TEXT:anab,offset _TEXT:anac	   ; a0,a1
	dw	offset _TEXT:anad,offset _TEXT:anae	   ; a2,a3
	dw	offset _TEXT:anah,offset _TEXT:anal	   ; a4,a5
	dw	offset _TEXT:anam,offset _TEXT:anaa	   ; a6,a7
	dw	offset _TEXT:xrab,offset _TEXT:xrac	   ; a8,a9
	dw	offset _TEXT:xrad,offset _TEXT:xrae	   ; aa,ab
	dw	offset _TEXT:xrah,offset _TEXT:xral	   ; ac,ad
	dw	offset _TEXT:xram,offset _TEXT:xraa	   ; ae,af
	dw	offset _TEXT:orab,offset _TEXT:orac	   ; b0,b1
	dw	offset _TEXT:orad,offset _TEXT:orae	   ; b2,b3
	dw	offset _TEXT:orah,offset _TEXT:oral	   ; b4,b5
	dw	offset _TEXT:oram,offset _TEXT:oraa	   ; b6,b7
	dw	offset _TEXT:cmpb,offset _TEXT:cmpc	   ; b8,b9
	dw	offset _TEXT:cmpd,offset _TEXT:cmpe	   ; ba,bb
	dw	offset _TEXT:cmph,offset _TEXT:cmpl	   ; bc,bd
	dw	offset _TEXT:cmpm,offset _TEXT:cmpa	   ; be,bf
	dw	offset _TEXT:rnz, offset _TEXT:popb	   ; c0,c1
	dw	offset _TEXT:jnz1,offset _TEXT:jmp1	   ; c2,c3
	dw	offset _TEXT:cnz, offset _TEXT:pushb	   ; c4,c5
	dw	offset _TEXT:adi, offset _TEXT:rst0	   ; c6,c7
	dw	offset _TEXT:rz,  offset _TEXT:ret1	   ; c8,c9
	dw	offset _TEXT:jz1, offset _TEXT:halt	   ; ca,cb
	dw	offset _TEXT:cz,  offset _TEXT:call1	   ; cc,cd
	dw	offset _TEXT:aci, offset _TEXT:rst1	   ; ce,cf
	dw	offset _TEXT:rnc, offset _TEXT:popd	   ; d0,d1
	dw	offset _TEXT:jnc1,offset _TEXT:halt	   ; d2,d3
	dw	offset _TEXT:cnc, offset _TEXT:pushd	   ; d4,d5
	dw	offset _TEXT:sui, offset _TEXT:rst2	   ; d6,d7
	dw	offset _TEXT:rc,  offset _TEXT:halt	   ; d8,d9
	dw	offset _TEXT:jc1, offset _TEXT:halt	   ; da,db
	dw	offset _TEXT:cc,  offset _TEXT:halt	   ; dc,dd
	dw	offset _TEXT:sbi, offset _TEXT:rst3	   ; de,df
	dw	offset _TEXT:rpo, offset _TEXT:poph	   ; e0,e1
	dw	offset _TEXT:jpo1,offset _TEXT:xthl	   ; e2,e3
	dw	offset _TEXT:cpo, offset _TEXT:pushh	   ; e4,e5
	dw	offset _TEXT:ani, offset _TEXT:rst4	   ; e6,e7
	dw	offset _TEXT:rpe, offset _TEXT:pchl	   ; e8,e9
	dw	offset _TEXT:jpe1,offset _TEXT:xchgx	   ; ea,eb
	dw	offset _TEXT:cpe, offset _TEXT:halt	   ; ec,ed
	dw	offset _TEXT:xri, offset _TEXT:rst5	   ; ee,ef
	dw	offset _TEXT:rp,  offset _TEXT:poppsw    ; f0,f1
	dw	offset _TEXT:jpx, offset _TEXT:fetch	   ; f2,f3
	dw	offset _TEXT:cp,  offset _TEXT:pushpsw   ; f4,f5
	dw	offset _TEXT:ori, offset _TEXT:rst6	   ; f6,f7
	dw	offset _TEXT:rm,  offset _TEXT:sphl	   ; f8,f9
	dw	offset _TEXT:jm,  offset _TEXT:fetch	   ; fa,fb
	dw	offset _TEXT:cm,  offset _TEXT:halt	   ; fc,fd
	dw	offset _TEXT:cpi, offset _TEXT:rst7	   ; fe,ff

_v20	db	0		; NEC V20 flag

_DATA	ends

; 8080 register equates

;a	 equ	 al
;flags	 equ	 ah
;psw	 equ	 ax
;b	 equ	 ch
;c	 equ	 cl
;bc	 equ	 bx
;d	 equ	 dh
;e	 equ	 dl
;de	 equ	 dx
;h	 equ	 bh
;l	 equ	 bl
;hl	 equ	 bx
;m	 equ	 byte ptr [bx]
;pc	 equ	 si
;scratch equ	 di

_TEXT	segment byte public 'CODE'

	extrn	_isis:near

; Macros for checking stray pointers - remove when simulator works

chkbx	macro
;	call	checkbx
	endm

chkdi	macro
;	call	checkdi
	endm

checkbx proc	near
	cmp	bx,0367fh
	ja	bxok
	int	3		; enter debugger
bxok:
	ret
checkbx endp

checkdi proc	near
	cmp	di,0367fh
	ja	diok
	int	3		; enter debugger
diok:
	ret
checkdi endp

; Macros for pushing and popping registers - at one time we used bp as stack

push1	macro	reg
	push	reg
;	sub	bp,2
;	mov	[bp],reg
	endm

pop1	macro	reg
	pop	reg
;	mov	reg,[bp]
;	add	bp,2
	endm

; Macro for getting the flags from where they are saved (ah for now).
; Must be used only in each instruction processing routine, not
; in the various instruction fetch/decode routines.

getf	macro
	sahf
	endm


; HALT - save registers and call ISIS-II emulator.
;
; This routine is called to emulate illegal instructions.  It
; is assumed that FD is used at the ISIS and monitor entry points
; to break emulation.  The FD must be followed by a C9 (RET).

halt:
	mov	_savepsw,ax
	mov	_savebc,cx
	mov	_savede,dx
	mov	_savehl,bx
	mov	_savesp,sp
	mov	_savepc,si
	mov	sp,offset dgroup:_stak	; use alternate stack
	call	_isis			; when doing ISIS stuff
					; drop into go8080

; GO8080 - goto 8080 program
;
; GO8080 can be used by external programs to resume emulation using the
; saved register values.

	public	_go8080

_go8080	proc	near
	mov	ax,_savepsw
	mov	cx,_savebc
	mov	dx,_savede
	mov	bx,_savehl
	mov	sp,_savesp
	mov	si,_savepc
	jmp	short fetch
_go8080	endp


; FETCH - fetch and decode the next 8080 instruction
;
; This is the main instruction fetch loop.
; There are six entry points:
;	FETCHF	update flags from previous operation, then fetch instruction
;	BUMP1F	bump pc by 1, then do a FETCHF
;	BUMP2F	bump pc by 2, then do a FETCHF
;	FETCH	discard flags, then fetch instruction
;	BUMP1	bump pc by 1, then do a FETCH
;	BUMP2	bump pc by 2, then do a FETCH
; The fetch operation destroys the flags (they're still in ah though),
; so any instruction that needs the flags back must do a "getf" macro call.

bump2f:
	lahf			; save flags
	add	si,2		; skip two-byte operand
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler
bump1f:
	lahf			; save flags
	inc	si		; skip past one-byte operand
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler

fetchf:
	lahf			; save flags
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler
bump2:
	add	si,2
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler
bump1:
	inc	si		; bump program counter by 1
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler

fetch:
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler

; 1-f

lxib:	mov	cx,[si]
	jmp	bump2
staxb:	mov	di,cx
	chkdi
	mov	[di],al
	jmp	fetch
inxb:	inc	cx
	jmp	fetch
inrb:	getf
	inc	ch
	jmp	fetchf
dcrb:	getf
	dec	ch
	jmp	fetchf
mvib:	mov	ch,[si]
	jmp	bump1
	getf
rlc:	getf
	rol	al,1
	jmp	fetchf
dadb:	add	bx,cx
testc:	jc	stc1
clc1:	and	ah,0feh
	jmp	fetch
stc1:	or	ah,1
	jmp	fetch
ldaxb:	mov	di,cx
	mov	al,[di]
	jmp	fetch
dcxb:	dec	cx
	jmp	fetch
inrc:	getf
	inc	cl
	jmp	fetchf
dcrc:	getf
	dec	cl
	jmp	fetchf
mvic:	mov	cl,[si]
	jmp	bump1
rrc:	getf
	ror	al,1
	jmp	fetchf

; 10-1f

lxid:	mov	dx,[si]
	jmp	bump2
staxd:	mov	di,dx
	chkdi
	mov	[di],al
	jmp	fetch
inxd:	inc	dx
	jmp	fetch
inrd:	getf
	inc	dh
	jmp	fetchf
dcrd:	getf
	dec	dh
	jmp	fetchf
mvid:	mov	dh,[si]
	jmp	bump1
ral:	getf
	rcl	al,1
	jmp	fetchf
dadd:	add	bx,dx
	jmp	testc
ldaxd:	mov	di,dx
	mov	al,[di]
	jmp	fetch
dcxd:	dec	dx
	jmp	fetch
inre:	getf
	inc	dl
	jmp	fetchf
dcre:	getf
	dec	dl
	jmp	fetchf
mvie:	mov	dl,[si]
	jmp	bump1
rar:	getf
	rcr	al,1
	jmp	fetchf

; 20-2f

lxih:	mov	bx,[si]
	jmp	bump2
shldx:	mov	di,[si]
	chkdi
	mov	[di],bx
	jmp	bump2
inxh:	inc	bx
	jmp	fetch
inrh:	getf
	inc	bh
	jmp	fetchf
dcrh:	getf
	dec	bh
	jmp	fetchf
mvih:	mov	bh,[si]
	jmp	bump1
daa1:	getf
	daa
	jmp	fetchf
dadh:	add	bx,bx
	jmp	testc
lhld:	mov	di,[si]
	chkdi
	mov	bx,[di]
	jmp	bump2
dcxh:	dec	bx
	jmp	fetch
inrl:	getf
	inc	bl
	jmp	fetchf
dcrl:	getf
	dec	bl
	jmp	fetchf
mvil:	mov	bl,[si]
	jmp	bump1
cma:	not	al
	jmp	fetch

; 30-3F

lxisp:	mov	sp,[si]
	jmp	bump2
sta:	mov	di,[si]
	chkdi
	mov	[di],al
	jmp	bump2
inxsp:	inc	sp
	jmp	fetch
inrm:	chkbx
	getf
	inc	byte ptr [bx]
	jmp	fetchf
dcrm:	chkbx
	getf
	dec	byte ptr [bx]
	jmp	fetchf
mvim:	mov	di,bx
	chkdi
	mov	bl,[si]
	mov	[di],bl
	mov	bx,di
	jmp	bump1
dadsp:	add	bx,sp
	jmp	testc
lda:	mov	di,[si]
	mov	al,[di]
	jmp	bump2
dcxsp:	dec	sp
	jmp	fetch
inra:	getf
	inc	al
	jmp	fetchf
dcra:	getf
	dec	al
	jmp	fetchf
mvia:	mov	al,[si]
	jmp	bump1
cmc1:	xor	ah,1
	jmp	fetch

; 40-4F

movbb:	mov	ch,ch
	jmp	fetch
movbc:	mov	ch,cl
	jmp	fetch
movbd:	mov	ch,dh
	jmp	fetch
movbe:	mov	ch,dl
	jmp	fetch
movbh:	mov	ch,bh
	jmp	fetch
movbl:	mov	ch,bl
	jmp	fetch
movbm:	mov	ch,[bx]
	jmp	fetch
movba:	mov	ch,al
	jmp	fetch
movcb:	mov	cl,ch
	jmp	fetch
movcc:	mov	cl,cl
	jmp	fetch
movcd:	mov	cl,dh
	jmp	fetch
movce:	mov	cl,dl
	jmp	fetch
movch:	mov	cl,bh
	jmp	fetch
movcl:	mov	cl,bl
	jmp	fetch
movcm:	mov	cl,[bx]
	jmp	fetch
movca:	mov	cl,al
	jmp	fetch

; 50-5F

movdb:	mov	dh,ch
	jmp	fetch
movdc:	mov	dh,cl
	jmp	fetch
movdd:	mov	dh,dh
	jmp	fetch
movde:	mov	dh,dl
	jmp	fetch
movdh:	mov	dh,bh
	jmp	fetch
movdl:	mov	dh,bl
	jmp	fetch
movdm:	mov	dh,[bx]
	jmp	fetch
movda:	mov	dh,al
	jmp	fetch
moveb:	mov	dl,ch
	jmp	fetch
movec:	mov	dl,cl
	jmp	fetch
moved:	mov	dl,dh
	jmp	fetch
movee:	mov	dl,dl
	jmp	fetch
moveh:	mov	dl,bh
	jmp	fetch
movel:	mov	dl,bl
	jmp	fetch
movem:	mov	dl,[bx]
	jmp	fetch
movea:	mov	dl,al
	jmp	fetch

; 60-6F

movhb:	mov	bh,ch
	jmp	fetch
movhc:	mov	bh,cl
	jmp	fetch
movhd:	mov	bh,dh
	jmp	fetch
movhe:	mov	bh,dl
	jmp	fetch
movhh:	mov	bh,bh
	jmp	fetch
movhl:	mov	bh,bl
	jmp	fetch
movhm:	mov	bh,[bx]
	jmp	fetch
movha:	mov	bh,al
	jmp	fetch
movlb:	mov	bl,ch
	jmp	fetch
movlc:	mov	bl,cl
	jmp	fetch
movld:	mov	bl,dh
	jmp	fetch
movle:	mov	bl,dl
	jmp	fetch
movlh:	mov	bl,bh
	jmp	fetch
movll:	mov	bl,bl
	jmp	fetch
movlm:	mov	bl,[bx]
	jmp	fetch
movla:	mov	bl,al
	jmp	fetch

; 70-7F

movmb:	chkbx
	mov	[bx],ch
	jmp	fetch
movmc:	chkbx
	mov	[bx],cl
	jmp	fetch
movmd:	chkbx
	mov	[bx],dh
	jmp	fetch
movme:	chkbx
	mov	[bx],dl
	jmp	fetch
movmh:	chkbx
	mov	[bx],bh
	jmp	fetch
movml:	chkbx
	mov	[bx],bl
	jmp	fetch
movma:	chkbx
	mov	[bx],al
	jmp	fetch
movab:	mov	al,ch
	jmp	fetch
movac:	mov	al,cl
	jmp	fetch
movad:	mov	al,dh
	jmp	fetch
movae:	mov	al,dl
	jmp	fetch
movah:	mov	al,bh
	jmp	fetch
moval:	mov	al,bl
	jmp	fetch
movam:	mov	al,[bx]
	jmp	fetch
movaa:	mov	al,al
	jmp	fetch

; 80-8F

addb:	add	al,ch
	jmp	fetchf
addc:	add	al,cl
	jmp	fetchf
addd:	add	al,dh
	jmp	fetchf
adde:	add	al,dl
	jmp	fetchf
addh:	add	al,bh
	jmp	fetchf
addl:	add	al,bl
	jmp	fetchf
addm:	add	al,[bx]
	jmp	fetchf
adda:	add	al,al
	jmp	fetchf
adcb:	getf
	adc	al,ch
	jmp	fetchf
adcc:	getf
	adc	al,cl
	jmp	fetchf
adcd:	getf
	adc	al,dh
	jmp	fetchf
adce:	getf
	adc	al,dl
	jmp	fetchf
adch:	getf
	adc	al,bh
	jmp	fetchf
adcl:	getf
	adc	al,bl
	jmp	fetchf
adcm:	getf
	adc	al,[bx]
	jmp	fetchf
adca:	getf
	adc	al,al
	jmp	fetchf

; 90-9F

subb:
	sub	al,ch
	jmp	fetchf
subc:
	sub	al,cl
	jmp	fetchf
subd:
	sub	al,dh
	jmp	fetchf
sube:
	sub	al,dl
	jmp	fetchf
subh:
	sub	al,bh
	jmp	fetchf
subl:
	sub	al,bl
	jmp	fetchf
subm:
	sub	al,[bx]
	jmp	fetchf
suba:
	sub	al,al
	jmp	fetchf
sbbb:	getf
	sbb	al,ch
	jmp	fetchf
sbbc:	getf
	sbb	al,cl
	jmp	fetchf
sbbd:	getf
	sbb	al,dh
	jmp	fetchf
sbbe:	getf
	sbb	al,dl
	jmp	fetchf
sbbh:	getf
	sbb	al,bh
	jmp	fetchf
sbbl:	getf
	sbb	al,bl
	jmp	fetchf
sbbm:	getf
	sbb	al,[bx]
	jmp	fetchf
sbba:	getf
	sbb	al,al
	jmp	fetchf

; a0-aF

anab:	and	al,ch
	jmp	fetchf
anac:	and	al,cl
	jmp	fetchf
anad:	and	al,dh
	jmp	fetchf
anae:	and	al,dl
	jmp	fetchf
anah:	and	al,bh
	jmp	fetchf
anal:	and	al,bl
	jmp	fetchf
anam:	and	al,[bx]
	jmp	fetchf
anaa:	and	al,al
	jmp	fetchf
xrab:	xor	al,ch
	jmp	fetchf
xrac:	xor	al,cl
	jmp	fetchf
xrad:	xor	al,dh
	jmp	fetchf
xrae:	xor	al,dl
	jmp	fetchf
xrah:	xor	al,bh
	jmp	fetchf
xral:	xor	al,bl
	jmp	fetchf
xram:	xor	al,[bx]
	jmp	fetchf
xraa:	xor	al,al
	jmp	fetchf

; b0-bF

orab:	or	al,ch
	jmp	fetchf
orac:	or	al,cl
	jmp	fetchf
orad:	or	al,dh
	jmp	fetchf
orae:	or	al,dl
	jmp	fetchf
orah:	or	al,bh
	jmp	fetchf
oral:	or	al,bl
	jmp	fetchf
oram:	or	al,[bx]
	jmp	fetchf
oraa:	or	al,al
	jmp	fetchf
cmpb:	cmp	al,ch
	jmp	fetchf
cmpc:	cmp	al,cl
	jmp	fetchf
cmpd:	cmp	al,dh
	jmp	fetchf
cmpe:	cmp	al,dl
	jmp	fetchf
cmph:	cmp	al,bh
	jmp	fetchf
cmpl:	cmp	al,bl
	jmp	fetchf
cmpm:	cmp	al,[bx]
	jmp	fetchf
cmpa:	cmp	al,al
	jmp	fetchf

; c0 - cf

rnz:	getf
	jz	fetch1
	pop1	si
	jmp	short fetch1
popb:	pop1	cx
	jmp	short fetch1
jnz1:	getf
	jz	bump2a
jmp1:	mov	si,[si]
	jmp	short fetch1
cnz:	getf
	jnz	call1
	jmp	short bump2a
pushb:	push1	cx
	jmp	short fetch1
adi:	add	al,[si]
	jmp	bump1f
rst0:	xor	di,di
	jmp	short calldi1
rz:	getf
	jnz	fetch1
ret1:	pop1	si
	jmp	short fetch1
jz1:	getf
	jnz	bump2a
	mov	si,[si]
	jmp	short fetch1
cz:	getf
	jz	call1
	jmp	short bump2a
aci:	getf
	adc	al,[si]
	jmp	bump1f
rst1:	mov	di,8
	jmp	short calldi1

; routines used by c0-df

call1:
	mov	di,[si]
	inc	si
	inc	si
calldi1:			; come here for RSTx, di contains new PC
	push1	si
	mov	si,di
fetch1:
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler
bump2a:
	add	si,2		; skip past two-byte operand
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler

; d0 - df

rnc:	getf
	jc	fetch1
	pop1	si
	jmp	short fetch1
popd:	pop1	dx
	jmp	short fetch1
jnc1:	getf
	jc	bump2a
	mov	si,[si]
	jmp	short fetch1
cnc:	getf
	jnc	call1
	jmp	short bump2a
pushd:	push1	dx
	jmp	short fetch1
sui:	sub	al,[si]
	jmp	bump1f
rst2:	mov	di,2*8
	jmp	short calldi1
rc:	getf
	jnc	fetch1
	pop1	si
	jmp	short fetch1
jc1:	getf
	jnc	bump2a
	mov	si,[si]
	jmp	short fetch1
cc:	getf
	jc	call1
	jmp	short bump2a
sbi:	getf
	sbb	al,[si]
	jmp	bump1f
rst3:	mov	di,3*8
	jmp	short calldi2

; e0 - ef

rpo:	getf
	jpe	fetch2
	pop1	si
	jmp	short fetch2
poph:	pop1	bx
	jmp	short fetch2
jpo1:	getf
	jpe	bump2b
	mov	si,[si]
	jmp	short fetch2
xthl:	mov	bp,sp
	xchg	bx,[bp]
	jmp	short fetch2
cpo:	getf
	jpo	call2
	jmp	short bump2b
pushh:	push1	bx
	jmp	short fetch2
ani:	and	al,[si]
	jmp	bump1f
rst4:	mov	di,4*8
	jmp	short calldi2
rpe:	getf
	jpo	fetch2
	pop1	si
	jmp	short fetch2
pchl:	mov	si,bx
	jmp	short fetch2
jpe1:	getf
	jpo	bump2b
	mov	si,[si]
	jmp	short fetch2
xchgx:	xchg	dx,bx
	jmp	short fetch2
cpe:	getf
	jpe	call2
	jmp	short bump2b
xri:	xor	al,[si]
	jmp	bump1f
rst5:	mov	di,5*8
	jmp	short calldi2

; routines used by e0-ff

call2:
	mov	di,[si]
	inc	si
	inc	si
calldi2:			; come here for RSTx, di contains new PC
	push1	si
	mov	si,di
fetch2:
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler
bump2b:
	add	si,2		; skip past two byte operand
	mov	di,[si] 	; get the opcode
	inc	si		; bump the program counter
	and	di,0ffh 	; mask off high byte of opcode
	shl	di,1		; make it a word pointer
;	sahf			; restore the flags
	jmp	[di+jmptab]	; jump to appropriate instruction handler

; f0 - ff

rp:	getf
	js	fetch2
	pop1	si
	jmp	short fetch2
poppsw: pop1	ax
	xchg	ah,al
	jmp	short fetch2
jpx:	getf
	js	bump2b
	mov	si,[si]
	jmp	short fetch2
cp:	getf
	jns	call2
	jmp	short bump2b
pushpsw:xchg	ah,al
	push1	ax
	xchg	ah,al
	jmp	short fetch2
ori:	or	al,[si]
	jmp	bump1f
rst6:	mov	di,6*8
	jmp	short calldi2
rm:	getf
	jns	fetch2
	pop1	si
	jmp	short fetch2
sphl:	mov	sp,bx
	jmp	short fetch2
jm:	getf
	jns	bump2b
	mov	si,[si]
	jmp	short fetch2
cm:	getf
	js	call2
	jmp	short bump2b
cpi:	cmp	al,[si]
	jmp	bump1f
rst7:	mov	di,7*8
	jmp	short calldi2

_TEXT	ends

	end
