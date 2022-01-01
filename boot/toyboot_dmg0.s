; MIT License
;
; Copyright (c) 2021 Tyulis

; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to deal
; in the Software without restriction, including without limitation the rights
; to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
; copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:

; The above copyright notice and this permission notice shall be included in all
; copies or substantial portions of the Software.

; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
; OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
; SOFTWARE.


; DMG0 version of the base template

.define BOOT_VALUE_A $01
.define BOOT_VALUE_BC $FF13
.define BOOT_VALUE_DE $00C1
.define BOOT_VALUE_HL $8403

.define BOOT_VALUE_DIV $18
.define BOOT_VALUE_STAT $81

.define TIMING_ALIGNMENT $1110  ; Value found empirically

.include toyboot_template_dmg_fast.s


.org $00EF
handOff:
	ld b, $01
	xor a
	xor a, b  ; Reset flags
	ld bc, BOOT_VALUE_BC
	ld de, BOOT_VALUE_DE
	ld hl, BOOT_VALUE_HL
	ld a, BOOT_VALUE_A
	ldh ($50), a
