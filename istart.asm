	title	istart - ISIS startup file and C interface to some DOS 2.0 functions
	page	66,128
	name	start

_TEXT	segment byte public 'CODE'
_TEXT	ends

DGROUP	group	_DATA,_BSS,_STACK
	assume	cs:_TEXT,ds:DGROUP


; DATA and STACK segments

_DATA	segment para public 'DATA'

	public	_psp

isdata	db	50h dup (0)	; ISIS-II data - must be at locations 0-4F

_psp	db	256 dup (?)	; copy of the Program Segment Prefix
env	db	256 dup (?)	; copy of one environment string (getenv)

; Following is a buffer for DOS function 0A (buffered keyboard input)

count	db	?		; number of bytes requested
actual	db	?		; actual number of bytes returned by DOS
linebuf db	256 dup (?)	; the typed line itself

_DATA	ends

_BSS	segment word public 'BSS'
_BSS	ends

_TEXT	segment byte public 'CODE'
	extrn	_main:near

; START - set up a stack and call main

; Make a copy of the PSP in the C data segment (at "psp")
; DS:0 already points at the PSP.

start:
	cld
	mov	ax,DGROUP
	mov	es,ax
	mov	si,0
	mov	di,offset DGROUP:_psp
	mov	cx,256
rep	movsb

; Set up the SS and DS registers to point to the C data segment

	cli
	mov	ds,ax
	mov	ss,ax
	assume	ds:DGROUP

; Set up a 512-byte stack

	mov	sp,offset DGROUP:_stak
	sti

; Call the user's main program

	call	_main

; Do an exit if user program just dropped off the end his main function

	mov	ax,0		; if it returns, assume no error
	push	ax
	call	_exit

; CREAT - create a file and return its file descriptor
;
; char *name;			/* null-terminated filename */
; int fd;			/* file descriptor */
; int pmode;			/* file attributes */
; fd = creat(name,pmode);	/* -1 if error */

	public	_creat
_creat	proc	near
	push	bp
	mov	bp,sp
	mov	dx,[bp+4]	; get the name pointer
	mov	cx,[bp+6]	; get the file attributes
	mov	ah,03ch 	; call DOS function 3C
	int	21h
errorchk:
	jc	error
	pop	bp
	ret			; result is in AX
error:
	mov	ax,-1		; UNIX error return
	pop	bp
	ret
_creat	endp


; OPEN - open a file and return its file descriptor
;
; char *name;			/* null-terminated filename */
; int fd;			/* file descriptor */
; int rwmode;			/* 0 = read, 1 = write, 2 = read/write */
; fd = creat(name,wrmode);	/* -1 if error */

	public	_open
_open	proc	near
	push	bp
	mov	bp,sp
	mov	dx,[bp+4]	; get the name pointer
	mov	al,[bp+6]	; get the access code
	mov	ah,03dh 	; call DOS function 3D
	int	21h
	jmp	errorchk
_open	endp


; CLOSE - close a file
;
; int fd;
; close(fd);

	public	_close
_close	proc	near
	push	bp
	mov	bp,sp
	mov	bx,[bp+4]	; get the file descriptor
	mov	ah,03eh 	; call DOS function 3E
	int	21h
	jmp	errorchk
_close	endp


; READ - read data from a file
;
; int fd;
; char *buf;
; int n;
; n_read = read(fd,buf,n)	/* returns -1 if error */

	public	_read
_read	proc	near
	push	bp
	mov	bp,sp
	mov	bx,[bp+4]	; get file descriptor
	mov	dx,[bp+6]	; get buffer pointer
	mov	cx,[bp+8]	; get number of bytes to read
	cmp	bx,0		; is it the console input file?
	je	getcon		; yes - use buffered keyboard input call
	mov	ah,03fh 	; call DOS function 3F
	int	21h
	jmp	errorchk

; come here for reading from console input (file descriptor 0)

getcon:
	sub	cl,2		; decrement requested count by 2 (allow CRLF)
	mov	count,cl	; store requested count in buffer
	mov	dx,offset DGROUP:count	  ; address of buffer
	mov	ah,0ah		; call DOS function 0A
	int	21h
	mov	dl,0ah		; echo a line feed character
	mov	ah,2
	int	21h		; call DOS function 2 to output the linefeed
	mov	cl,actual	; get actual number of bytes read
	mov	ch,0		; clear high byte of count
	mov	si,offset DGROUP:linebuf ; get address of returned characters
	mov	di,[bp+6]	; get address of caller's buffer
	mov	ax,ds		; copy DS to ES
	mov	es,ax
	cld
rep	movsb			; copy bytes to user's buffer
	mov	al,0dh		; append a CR/LF
	stosb
	mov	al,0ah
	stosb
	mov	al,actual	; get the count back again
	mov	ah,0
	add	ax,2		; return the actual number of bytes read
	pop	bp
	ret
_read	endp


; WRITE - write data to a file
;
; int fd;
; char *buf;
; int n;
; n_written = write(fd,buf,n)	/* returns -1 if error */

	public	_write
_write	proc	near
	push	bp
	mov	bp,sp
	mov	bx,[bp+4]	; get file descriptor
	mov	dx,[bp+6]	; get buffer pointer
	mov	cx,[bp+8]	; get number of bytes to write
	mov	ah,040h 	; call DOS function 40
	int	21h
	jmp	errorchk
_write	endp


; EXIT - exit to DOS
;
; int errorcode;
; exit(errorcode);

	public	_exit
_exit	proc	near
	push	bp
	mov	bp,sp
	mov	al,[bp+4]	; get error code
	mov	ah,04ch 	; call DOS function 4C
	int	21h
_exit	endp


; RENAME - rename a file
;
; char *oldname, *newname;
; int errorcode;
; errorcode = rename(oldname,newname);

	public 	_rename
_rename	proc	near
	push	bp
	mov	bp,sp
	mov	dx,[bp+4]	; get old name
	mov	si,[bp+6]	; get new name
	mov	ax,ds		; copy ds to es
	mov	es,ax
	mov	ah,056h 	; call DOS function 56
	int	21h
	jmp	errorchk	; go check for errors
_rename	endp


; UNLINK - delete a file
;
; char *filename;
; int errorcode;
; errorcode = unlink(filename);

	public	_unlink
_unlink	proc	near
	push	bp
	mov	bp,sp
	mov	dx,[bp+4]	; get name of file to delete
	mov	ah,041h 	; call DOS function 41
	int	21h
	jmp	errorchk	; go check for errors
_unlink	endp


; LSEEK - change file position
;
; int fd;
; long offset;
; int origin;	/* 0 = start of file, 1 = current position, 2 = end of file */
;
; offset = lseek(fd,offset,origin)  /* returns new offset */

	public	_lseek
_lseek	proc	near
	push	bp
	mov	bp,sp
	mov	bx,[bp+4]	; get file handle
	mov	dx,[bp+6]	; get the low part of offset
	mov	cx,[bp+8]	; get the high part of offset
	mov	al,[bp+10]	; get origin value
	mov	ah,042h 	; call DOS function 42
	int	21h
	jnc	lseekok
	mov	ax,-1		; return FPOS of -1 if error
	mov	bx,ax
	pop	bp
	ret
lseekok:
;	mov	bx,ax		; return FPOS in ax:bx for C programs
;	mov	ax,dx
	pop	bp
	ret
_lseek	endp


; GETENV - get the value of an environment string
;
; char *getenv(char *name);
; returns 0 if the name can't be found.

	public	_getenv
_getenv	proc	near
	push	bp
	mov	bp,sp
	push	si			; save register variables
	push	di
	mov	ax,word ptr _psp+2ch	; get pointer to environment
	or	ax,ax			; is it NULL?
	jz	fail			; yes - program has no env.
	mov	es,ax
	xor	di,di			; es:di points to start of env.
	mov	ax,di			; use 0 in ax for various purposes
envloop:
	mov	cx,256			; maximum possible string
	mov	si,4[bp]		; get string we're looking for
repe	cmpsb
	cmp	es:[di-1],al		; at end of environment?
	jz	fail			; failure
	cmp	[si-1],al		; possible match?
	jnz	nextstr			; no - look at next string
	cmp	byte ptr es:[di-1],'='	; is it a match?
	jz	matched			; yes - go make a copy of value
nextstr:
	mov	cx,-1			; maximum search length
repne	scasb				; search for null (next string)
	jmp	envloop
matched:
	mov	si,offset DGROUP:env	; copy string to 'env'
matchloop:
	mov	al,es:[di]		; get a byte
	mov	[si],al			; save it
	inc	si			; bump source pointer
	inc	di			; bump dest pointer
	or	al,al			; was it a null?
	jnz	matchloop		; no - copy more
	mov	ax,offset DGROUP:env	; return address of copy
envret:
	pop	di			; restore register variables
	pop	si
	pop	bp
	ret
fail:
	xor	al,al
	jmp	envret
_getenv	endp

; BDOS - perform simple DOS calls
;
; int bdos(ah,dx,al)	- returns value in al

	public	_bdos
_bdos	proc	near
	push	bp
	mov	bp,sp
	mov	ah,[bp+4]	; get function number
	mov	dx,[bp+6]	; get value for dx
	mov	al,[bp+8]	; get value for al
	int	21h		; call DOS
	xor	ah,ah		; clear high byte of result
	pop	bp
	ret
_bdos	endp

_TEXT	ends

_STACK	segment word public 'DATA'
	public	_stak

junk	db	512 dup (?)	; stack space
_stak	dw	?		; top of stack marker

_STACK	ends

	end	start
