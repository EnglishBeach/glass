Ideal
%nomacs

;
;ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออธ
;บ                                                                        ณ
;บ             COPYRIGHT (c) by Green Arrow Inc. 1990                     ณ
;บ                                                                        ณ
;บ                       All right reserved.                              ณ
;บ                                                                        ณ
;ำฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤฤู
;
;

Model	Large,C
	DosSeg

Global	wbkgnd	:Proc
Global	wcls	:Proc
Global	wgettext:Proc
Global	wputtext:Proc
Extrn	wwidth	:Proc
Extrn	wgeta	:Proc


	CodeSeg

;++
;
; ** _wbkgnd *-- Set attribute for rectangular window
;
; Call from Borland Turbo C:
;	wbkgnd (int x, int y, int dx, int dy, int attr);
;
; Fills rectangular area with left top corner in (x,y) and
; with heigth of dy and width of dx with text
; attribute of attr. Works properly on CGA & MGAin any text
; modes and EGA, EGAP & VGA in 80x25 & 40&25 modes.
; Requires modules from WSERV.C. Model is assumed Large only.
;
;--

Proc	wbkgnd	@x:word,@y:word,@dx:word,@dy:word,@attr:byte
Uses	si,di

	call	wwidth			; Get screen width
	shl	ax,1			;in AX, double it and
	push	ax			;save for future use.

	push	[@y]			; Obtain screen memory
	push	[@x]			;address as DX:AX.
	call	wgeta			;
	add	sp,4			; Clean stack.
	mov	es,dx			; Set proper ES.

	pop	dx			; Restore width.
	mov	bx,ax			; Screen pointer.
	inc	bx			;
	mov	si,[@dy]      		; Window heigth
	mov	al,[@attr]		; Attribute to store.
	cld				; Force fill forward.

; Loop for filling window

$10:	mov	di,bx			; Get initial pointer.
	mov	cx,[@dx]		; Window width.
$20:	stosb				; Fill in byte.
	inc	di			;
	loop	$20			;
	add	bx,dx			; Increase pointer.
	dec	si			; Check for done.
	jnz	$10			;

	ret				;

Endp					;

;+
;
; ** _wcls *-- Clear and set attribute for rectangular window
;
; Call from Borland Turbo C:
;	wcls (int x, int y, int dx, int dy, int attr);
;
; Clears rectangular area with left top corner in (x,y) and
; with heigth of dy and width of dx and fills it with text
; attribute of attr. Works properly on CGA & MGAin any text
; modes and EGA, EGAP & VGA in 80x25 & 40&25 modes.
; Requires modules from WSERV.C. Model is assumed Large only.
;
;--

Proc	wcls	@x:word,@y:word,@dx:word,@dy:word,@attr:byte
Uses	si,di

	call	wwidth			; Get screen width
	shl	ax,1			;in AX, double it and
	push	ax			;save for future use.

	push	[@y]			; Obtain screen memory
	push	[@x]			;address as DX:AX.
	call	wgeta 			;
	add	sp,4			; Clean stack.
	mov	es,dx			; Set proper ES.

	pop	dx			; Restore width.
	mov	bx,ax			; Screen pointer.
	mov	si,[@dy]  		; Window heigth
	mov	al,' '			; Space to store.
	mov	ah,[@attr] 		; Attributr to store.
	cld				; Force fill forward.

; Loop for filling window

$30:	mov	di,bx			; Get initial pointer.
	mov	cx,[@dx] 		; Window width.
	rep	stosw			; Fill in line.
	add	bx,dx			; Increase pointer.
	dec	si			; Check for done.
	jnz	$30			;

	ret				;

Endp


;++
;
; ** _wgettext *- Get text & attributes from rectanglr area
;
; Call from Borland Turbo C:
;	wgettext (int x, int y, int dx, int dy, char *buffer);
;
;--

Proc	wgettext @x:word,@y:word,@dx:word,@dy:word,@bfr:dataptr
Uses	si,di,ds

	call	wwidth	 		; Get screen width
	shl	ax,1			;in AX, double it and
	push	ax			;save for future use.

	push	[@y]			; Obtain screen memory
	push	[@x]			;address as DX:AX.
	call	wgeta			;
	add	sp,4			; Clean stack.
	mov	ds,dx			; Set proper ES.

	pop	dx			; Restore width.
	mov	bx,ax			; Screen pointer.
	mov	ax,[@dy] 		; Window heigth
	les	di,[@bfr]		;
	cld				; Force fill forward.

; Loop for filling window

$40:	mov	si,bx			; Get initial pointer.
	mov	cx,[@dx]		; Window width.
	rep	movsw			; Fill in line.
	add	bx,dx			; Increase pointer.
	dec	ax			; Check for done.
	jnz	$40			;

	ret

Endp

;++
;
; ** _wputtext *- Put text & attributes into rectanglr area
;
; Call from Borland Turbo C:
;	wputtext (int x, int y, int dx, int dy, char *buffer);
;
;--

Proc	wputtext @x:word,@y:word,@dx:word,@dy:word,@bfr:dataptr
Uses	si,di,ds

	call	wwidth	 		; Get screen width
	shl	ax,1			;in AX, double it and
	push	ax			;save for future use.

	push	[@y]			; Obtain screen memory
	push	[@x]	     		;address as DX:AX.
	call	wgeta 			;
	add	sp,4			; Clean stack.
	mov	es,dx			; Set proper ES.

	pop	dx			; Restore width.
	mov	bx,ax			; Screen pointer.
	mov	ax,[@dy]		; Window heigth
	lds	si,[@bfr]		; Buffer address
	cld				; Force fill forward.

; Loop for filling window

$50:	mov	di,bx			; Get initial pointer.
	mov	cx,[@dx]		; Window width.
	rep	movsw			; Fill in line.
	add	bx,dx			; Increase pointer.
	dec	ax			; Check for done.
	jnz	$50			;

	ret				;

Endp

End
