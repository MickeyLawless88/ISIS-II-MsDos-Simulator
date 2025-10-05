	title	movb - movb and setb for small model Datalight or Lattice C
	page	66,128
	name	movb

_TEXT	segment byte public 'CODE'
	assume	cs:_TEXT

; movb - move bytes
; char *source;			/* source buffer */
; char *dest;			/* destination buffer */
; int size;			/* no. of bytes to move */
; movb(source,dest,size);

	public	_movb
_movb	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
	mov	si,[bp+4]	; get the source pointer
	mov	di,[bp+6]	; get the dest pointer
	mov	cx,[bp+8]	; get the count
	mov	ax,ds		; copy ds to es
	mov	es,ax
	cld
rep	movsb			; move the bytes
	pop	di
	pop	si
	pop	bp
	ret
_movb	endp

; setb - fill bytes
; char value;			/* value to fill */
; char *buf;			/* buffer to fill */
; int size;			/* no. of bytes to fill */
; setb(source,dest,size);

	public	_setb
_setb	proc	near
	push	bp
	mov	bp,sp
	push	si
	push	di
	mov	al,[bp+4]	; get the value
	mov	di,[bp+6]	; get the buf pointer
	mov	cx,[bp+8]	; get the count
	mov	dx,ds		; copy ds to es
	mov	es,dx
	cld
rep	stosb			; fill the bytes
	pop	di
	pop	si
	pop	bp
	ret
_setb	endp

_TEXT	ends

	end
