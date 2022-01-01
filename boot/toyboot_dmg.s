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


; DMG version of the base template

.define BOOT_VALUE_A $01
.define BOOT_VALUE_BC $0013
.define BOOT_VALUE_DE $00D8
.define BOOT_VALUE_HL $014D

.define BOOT_VALUE_DIV $AB
.define BOOT_VALUE_STAT $81

.define TIMING_ALIGNMENT $0778  ; Value found empirically

.include toyboot_template_dmg_fast.s

.org $00F0
handOff:
	; We must setup the flags correctly (1000 if the header checksum is zero, 1011 otherwise)
	xor a
	ld hl, $014D
	sub a, (hl)
	add a, (hl)
end:
	ld bc, BOOT_VALUE_BC
	ld de, BOOT_VALUE_DE
	ld a, BOOT_VALUE_A
	ldh ($50), a
