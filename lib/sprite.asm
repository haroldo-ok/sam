.RAMSECTION "Sprite Data" SLOT 3
hw_sprites_y	dsb		64
hw_sprites_xc	dsw		64
.ENDS

.SECTION "Sprite Update Routine" FREE
UpdateSprites:
    ;==============================================================
    ; Sets the sprites' positions/attributes
    ;==============================================================
	;vdp set addr (Y table)
	
.MACRO WAIT_VRAM
	nop
	nop
	nop
.ENDM

	WAIT_VRAM
	xor	a
	out	($bf), a
	WAIT_VRAM
	ld	a, $7f
	out	($bf), a
	
	; Outputs Y table
	ld	hl, hw_sprites_y
	ld	bc, $40BE	; 64 bytes to $be
UpdateSprites_Y_Looop:
	WAIT_VRAM
	outi			; Output table
	ld a, b
	or a
	jr nz, UpdateSprites_Y_Looop

	;vdp set addr (X/Tile table)
	WAIT_VRAM
	ld	a, $80
	out	($bf), a
	WAIT_VRAM
	ld	a, $7f
	out	($bf), a
	
	; Outputs XA table
	ld	hl, hw_sprites_xc
	ld	bc, $80BE	; 128 bytes to $be
UpdateSprites_X_Loop:	
	WAIT_VRAM
	outi			; Output table
	ld a, b
	or a
	jr nz, UpdateSprites_X_Loop

    ret
.ENDS