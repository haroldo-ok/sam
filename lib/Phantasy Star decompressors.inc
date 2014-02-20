; Phantasy Star tile/tilemap decompression routines
;
; WLA DX warning:
;
;  Great as it is, sometimes WLA DX is buggy, and I've especially found
;  this with anonymous labels. If you're gettng screwy results, a
;  temporary fix is often to change the anonymous labels (-, +, __, etc)
;  to named labels.
;
; Usage notes:
;
; Tile loaders:
;
;   ld de,<target VRAM address ORed with $4000>
;   ld hl,<data address>
;   call LoadTiles4BitRLE
;
;   or
;
;   ld de,<target VRAM address ORed with $4000>
;   ld hl,<data address>
;   call LoadTiles4BitRLENoDI
;
;   The NoDI one does not di/ei around the VRAM accesses, so it's a bit
;   faster but will be messed up by VBlanks.
;
; Tilemap loader 1:
;
;   You need to define a constant called TileMapData. This must be a RAM
;   address of a copy of the tilemap which you'll have to copy into VRAM
;   during the VBlank.
;
;   ld hl,<data address>
;   call LoadTilemapToTileMapData
;
;   I haven't supplied the code to do the copying from RAM to VRAM.
;
; Tilemap loader 2:
;
;   ld de,<target VRAM address ORed with $4000>
;   ld hl,<data address>
;   call LoadTilemapToVRAM
;
;   This is one I wrote myself :) based on the tile loader. It loads the
;   tilemap directly into VRAM. It does not offer a version with di/ei
;   around VRAM accesses, because updating the tilemap during the active
;   display is generally bad; if you really want, you can modify it to
;   do that.
;
; Note:
;
;   Phantasy Star locates its SetVRAMAddressToDE function at offset
;   $0008, so it can do
;     rst $08                   (1 byte, 11 clock cycles)
;   instead of
;     call SetVRAMAddressToDE   (3 bytes, 17 clock cycles)
;   If your code is using that then either define UseRst yourself, or
;   uncomment the next line.

;.define UseRst

.section "SetVRAMAddressToDE" free
SetVRAMAddressToDE:
  ld a,e
  out ($bf),a
  ld a,d
  out ($bf),a
  ret
.ends

.section "Tile loader (4 bpp RLE, no di/ei)" free
; Decompresses tile data from hl to VRAM address de
LoadTiles4BitRLENoDI:
  ld b,$04
-:push bc
    push de
      call _f ; called 4 times for 4 bitplanes
    pop de
    inc de
  pop bc
  djnz -
  ret

_NotAnonymous:
__:
  ld a,(hl)          ; read count byte <----+
  inc hl             ; increment pointer    |
  or a               ; return if zero       |
  ret z              ;                      |
                     ;                      |
  ld c,a             ; get low 7 bits in b  |
  and $7f            ;                      |
  ld b,a             ;                      |
  ld a,c             ; set z flag if high   |
  and $80            ; bit = 0              |
                     ;                      |
-:                   ;                      |
.ifdef UseRst        ; SetVRAMAddressToDE<+ |
  rst $08            ;                    | |
.else                ;                    | |
  call SetVRAMAddressToDE ;               | |
.endif               ;                    | |
  ld a,(hl)          ; Get data byte in a | |
  out ($be),a        ; Write it to VRAM   | |
  jp z,+             ; If z flag then  -+ | |
                     ; skip inc hl      | | |
  inc hl             ;                  | | |
                     ;                  | | |
+:inc de             ; Add 4 to de <----+ | |
  inc de             ;                    | |
  inc de             ;                    | |
  inc de             ;                    | |
  djnz -             ; repeat block  -----+ |
                     ; b times              |
  jp nz,_b           ; If not z flag -------+
  inc hl             ; inc hl here instead  |
  jp _b              ; repeat forever ------+
                     ; (zero count byte quits)
.ends

.section "Phantasy Star Tile loader (4 bpp RLE, with di/ei)" free
LoadTiles4BitRLE:    ; Same as NoDI only with a di/ei around the VRAM access (because VBlanks will mess it up)
  ld b,$04           ; 4 bitplanes
-:push bc
    push de
      call _f ; called 4 times for 4 bitplanes
    pop de
    inc de
  pop bc
  djnz -
  ret

__:
  ld a,(hl)          ; header byte
  inc hl             ; data byte
  or a
  ret z              ; exit at zero terminator
  ld c,a             ; c = header byte
  and $7f
  ld b,a             ; b = count
  ld a,c
  and $80            ; z flag = high bit
-:di
.ifdef UseRst
  rst $08
.else
  call SetVRAMAddressToDE
.endif
  ld a,(hl)
  out ($be),a        ; output data
  ei
  jp z,+             ; if z flag then don't move to next data byte
  inc hl
+:inc de             ; move target forward 4 bytes
  inc de
  inc de
  inc de
  djnz -             ; repeat b times
  jp nz,_b
  inc hl
  jp _b
.ends

.section "Decompress to TileMapData" free
; Copies data from (hl) to TileMapData
; with RLE decompression and 2-interleaving
; data format:
; Header: $fccccccc
;   f = flag: 1 = not RLE, 0 = RLE
;   ccccccc = count
; Then [count] bytes are copied to even bytes starting at TileMapData
; Then the process is repeated for the odd bytes
LoadTilemapToTileMapData:
  ld b,$00           ; b=0
  ld de,TileMapData
  call _f            ; Process even bytes first -------------+
  inc hl             ; and odd bytes second                  |
  ld de,TileMapData+1 ;                                      |
__:ld a,(hl)         ; Get data count in a <-----------------+
  or a               ; \ return                              |
  ret z              ; / if zero                             |
  jp m,+             ; if bit 8 is set then ---------------+ |
                     ; else:                               | |
  ld c,a             ; put it in c -> bc = data count      | |
  inc hl             ; move hl pointer to next byte (data) | |
-:ldi                ; copy 1 byte from hl to de, <------+ | |
                     ; inc hl, inc de, dec bc            | | |
  dec hl             ; move hl pointer back (RLE)        | | |
  inc de             ; skip dest byte                    | | |
  jp pe,-            ; if bc!=0 then repeat -------------+ | |
  inc hl             ; move past RLE'd byte                | |
  jp _b              ; repeat -----------------------------|-+
+:and $7f            ; (if bit 8 is set:) unset it <-------+ |
  ld c,a             ; put it in c -> bc = data count        |
  inc hl             ; move hl pointer to next byte (data)   |
-:ldi                ; copy 1 byte from hl to de, <--------+ |
                     ; inc hl, inc de, dec bc              | |
  inc de             ; skip dest byte                      | |
  jp pe,-            ; if bc!=0 then repeat ---------------+ |
  jp _b              ; repeat -------------------------------+
.ends

.section "Decompress tilemap to VRAM" free
; Decompresses tilemap data from hl to VRAM address de
; This isn't from Phantasy Star; it's the tile routine modified for
; interleaving 2 instead of 4
LoadTilemapToVRAM:
  push de
    call _f
  pop de
  inc de
  call _f
  ret

__:
  ld a,(hl)          ; read count byte <----+
  inc hl             ; increment pointer    |
  or a               ; return if zero       |
  ret z              ;                      |
                     ;                      |
  ld c,a             ; get low 7 bits in b  |
  and $7f            ;                      |
  ld b,a             ;                      |
  ld a,c             ; set z flag if high   |
  and $80            ; bit = 0              |
                     ;                      |
-:di                 ;                      |
.ifdef UseRst        ; SetVRAMAddressToDE<+ |
  rst $08            ;                    | |
.else                ;                    | |
  call SetVRAMAddressToDE ;               | |
.endif               ;                    | |
  ld a,(hl)          ; Get data byte in a | |
  out ($be),a        ; Write it to VRAM   | |
  ei
  jr z,+             ; If z flag then  -+ | |
                     ; skip inc hl      | | |
  inc hl             ;                  | | |
                     ;                  | | |
+:inc de             ; Add 2 to de <----+ | |
  inc de             ;                    | |
  djnz -             ; repeat block  -----+ |
                     ; b times              |
  jr nz,_b           ; If not z flag -------+
  inc hl             ; inc hl here instead  |
  jr _b              ; repeat forever ------+
                     ; (zero count byte quits)
.ends

; Obs: For now, PSG decompressor has been pasted at the end of this file in order to avoid having to change the include path of existing projects.
; When the project file generation gets implemented, the code below will be able to be moved into its own include file.

; Phantasy Star Gaiden tile decompressor
; To use:
;
; .define PSGDecoderBuffer $c000                   ; define a 34 byte work area for it
; .include "Phantasy Star Gaiden decompressor.inc" ; include this file
; ld ix, <address of compressed data>
; ld hl, <VRAM address to write to, ORed with $4000>
; call PSG_decompress
;

.section "PSG decompressor" free

; .define PSG_vram_ptr PSGDecoderBuffer          ; word: VRAM address
; .define buffer PSGDecoderBuffer + 2            ; 32-byte decompression buffer

; hl = dest
; ix = src
PSG_decompress:
  ld (PSG_vram_ptr),hl  ; cache VRAM address
  ld c,(ix+0)    ; bc = number of tiles
  inc ix
  ld b,(ix+0)
  inc ix

_DecompressTile:
  push bc        ; save number of tiles
    ld b,$04     ; count 4 bitplanes
    ld de,PSG_buffer ; write to de
    ld c,(ix+0)  ; c = encoding information for 4 bitplanes
    inc ix

_DecompressBitplane:
    rlc c        ; %0x = all bits either 0 or 1
    jr nc,_AllTheSame
    rlc c        ; %11 = raw data
    jr c,_RawData

_Compressed:
    ld a,(ix+0)  ; get method byte
    inc ix

    ex de,hl     ; get bitplane, if it's referring to one
    ld d,a
    and $03
    add a,a      ; calculate address of that bitplane
    add a,a      ; = buffer + bitplane * 8
    add a,a
    ld e,a
    ld a,d       ; get method byte back
    ld d,$00
    ld iy,PSG_buffer
    add iy,de    ; now iy points to the referred to bitplane
    ex de,hl

    ; now check the method byte
    cp $03       ; %000000pp
    jr c,_DuplicateBitplane
    cp $10
    jr c,_CommonValue
    cp $13       ; %000100pp
    jr c,_DuplicateBitplaneInvert
    cp $20
    jr c,_CommonValue
    cp $23       ; %001000pp
    jr c,_DuplicateBitplanePartial
    cp $40
    jr c,_CommonValue
    cp $43       ; %010000pp
    jr c,_DuplicateBitplanePartialInvert
    ; fall through

_CommonValue:
    ld h,a       ; h = bitmask
    ld l,(ix+0)  ; l = common value
    inc ix
    jr _OutputCommonValue

_RawData:
    ld h,$00     ; empty bitmask; no common value
    jr _OutputCommonValue

_AllTheSame:
    rlc c        ; get next bit into carry
    sbc a,a      ; will make $00 if carry = 0, $ff if it's 1
    ld l,a       ; that's the common value
    ld h,$ff     ; full bitmask
    ; fall through

_OutputCommonValue:
    push bc
      ld b,8     ; loop counter
-:    ld a,l     ; get common value
      rlc h      ; get bit out of bitmask
      jr c,+     ; if 1, use the common value
      ld a,(ix+0); else get it from (ix++)
      inc ix
+:    ld (de),a  ; write to dest
      inc de
      djnz -     ; loop over 8 bytes
    pop bc
  jr _BitplaneDone

_DuplicateBitplane:
    ld hl,$ff00  ; full copy bitmask, empty inversion bitmask
    jr _OutputDuplicate

_DuplicateBitplaneInvert:
    ld hl,$ffff  ; full copy bitmask, full inversion bitmask
    jr _OutputDuplicate

_DuplicateBitplanePartial:
    ld h,(ix+0)  ; get copy bitmask
    ld l,$00     ; empty inversion bitmask
    inc ix
    jr _OutputDuplicate

_DuplicateBitplanePartialInvert:
    ld h,(ix+0)  ; get copt bitmask
    ld l,$ff     ; full inversion bitmask
    inc ix
    ; fall through

_OutputDuplicate:
    push bc
      ld b,8     ; loop counter
-:    ld a,(iy+0); read byte to copy
      inc iy
      xor l      ; apply inversion mask
      rlc h      ; get bit out of bitmask
      jr c,+     ; if 1, use the copied value
      ld a,(ix+0); else get it from (ix++)
      inc ix
+:    ld (de),a  ; write to dest
      inc de
      djnz -     ; loop over 8 bytes
    pop bc
    ; fall through

_BitplaneDone:
    dec b        ; decrement bitplane counter
    jp nz,_DecompressBitplane ; loop if not zero

_OutputTileToVRAM:
    ld a,(PSG_vram_ptr)
    out ($bf),a
    ld a,(PSG_vram_ptr+1)
    out ($bf),a

    ld de,$0008  ; we are interleaving every 8th byte
    ld c,e       ; counter for the interleaving run
    ld hl,PSG_buffer ; point at data to write

--: ld b,4       ; there are 4 bytes to interleave
    push hl
-:    ld a,(hl)  ; read byte
      out ($be),a; write to vram
      add hl,de  ; skip 8 bytes
      djnz -
    pop hl
    inc hl       ; next interleaving run
    dec c
    jr nz,--

    ; Add 32 bytes to PSG_vram_ptr
    ld hl,(PSG_vram_ptr)
    ld bc,32
    add hl,bc
    ld (PSG_vram_ptr),hl

  pop bc
  dec bc         ; next tile
  ld a,b
  or c
  jp nz,_DecompressTile
  ret            ; done

.ends
