Ideal
%nomacs

;
;ÉÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍÍ¸
;º                                                                        ³
;º             COPYRIGHT (c) by Green Arrow Inc. 1990                     ³
;º                                                                        ³
;º                       All right reserved.                              ³
;º                                                                        ³
;ÓÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ
;

Model	Large,C
	DosSeg


Public	load_font

	CodeSeg

;++
;
; ** _load_font *-- Load hardware character generator with specific font
;
; Call from Borland Turbo C:
;	load_font (int video_card);
;
; Loads proper font into video card hardware character generator.
;
;   video_card    action
;   ÄÄÄÄÄÄÄÄÄÄ    ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ
;        1        Loads EGA 8x14 font
;	 2        Loads VGA or 8514A VGA mode 8x16 font
;      other      No action done at all, merely returns
;
;--

Proc	load_font	@card:word
Uses	es


	mov	ax,[@card]		; Obtain card code
	lea	bp,[Egafnt]		; Assume EGA font
	mov	bh,14			; Font height
	cmp	ax,1			; Is it EGA font need to be loaded?
	je	$10			; Yes, do it
	cmp	ax,2			; Is it VGA font needed?
	jne	$20			; No, don't do anything
	lea	bp,[Vgafnt]		; It will be VGA font
	mov	bh,16			; Font height

$10:	mov	ax,@data		; Load ES with proper segment
	mov	es,ax			;address
	mov	ax,1100h		; AH=11 load user font
	xor	bl,bl			;
	mov	cx,0100h		; # of char
	mov	dx,0000h		; Starting char #
	int	10h			; Do

$20:	ret				;

	DataSeg

Egafnt:
	include	"ega_ch.inc"		;

Vgafnt:
	include	"vga_ch.inc"		;

Endp

end
