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


; This is intended to be an open-source bootstrap ROM for the DMG, DMG0 and MGB
; It does not perform any particular check on the ROM :
; It does not check the logo, nor the header checksum or whatever
; This is not intended to be accurate or anything, just to set things up more or less as they should be
; Here the boot_hwio and boot_regs tests from the Mooneye-GB test suite were taken
; as references for initial register values and timing-related values
; https://github.com/Gekkio/mooneye-test-suite/
; This stub bootrom makes ToyGB pass those two tests, that does not mean it's cycle-accurate
; But at least it's a relatively good timing within a few dozens CPU cycles.


start:
	ld sp, $FFFE

clearVRAM:
	xor a
	ld hl, $9FFF
clearVRAM.loop:
	ldd (hl), a
	bit 7, h               ; Stop when h goes from $80 to $7F
	jr nz, clearVRAM.loop

setupIO:
	ld a, $91
	ldh ($40), a   ; LCDC

	xor a
	ldh ($00), a   ; JOYP
	ldh ($01), a   ; SB
	ldh ($02), a   ; SC
	ldh ($05), a   ; TIMA
	ldh ($06), a   ; TMA
	ldh ($07), a   ; TAC
	ldh ($41), a   ; STAT
	ldh ($42), a   ; SCY
	ldh ($43), a   ; SCX
	ldh ($45), a   ; LYC
	ldh ($4A), a   ; WY
	ldh ($4B), a   ; WX
	ldh ($FF), a   ; IE

	inc a          ; $01
	ldh ($0F), a   ; IF

	ld bc, TIMING_ALIGNMENT   ; Value found empirically for each model
setupIO.waitForAlign:
	; This will reset DIV at the right moment to get STAT, LY and DIV align more or less properly at game startup
	; This is accurate enough to pass mooneye-gb's boot-hwio test, but it is not cycle-perfect
	dec c
	jr nz, setupIO.waitForAlign
	dec b
	jr nz, setupIO.waitForAlign

	ldh ($04), a

setupIO.waitForDIV:
	; Wait till the DIV register has the right value at startup (approximately)
	; It's done exactly at this moment to get at least some kind of minimal
	; accuracy (enough to pass mooneye-gb's boot-hwio test), but it is not cycle-perfect
	ldh a, ($04)
	cp BOOT_VALUE_DIV
	jr nz, setupIO.waitForDIV

	ld a, $80
	ldh ($26), a   ; NR52
	ld hl, $FF23
setupIO.clearAudio:
	ldd (hl), a
	bit 4, l  ; Stop when l goes from $10 to $0F
	jr nz, setupIO.clearAudio
setupIO.setupAudio:  ; Setup the audio channels that are used normally
	ld a, $77
	ldh ($24), a   ; NR50
	ld a, $F3
	ldh ($25), a   ; NR51
setupIO.playSound:  ; Channel 1 needs to be playing on game startup, so we need to play a sound
	ld a, $80
	ldh ($11), a   ; NR11
	ld a, $11      ; Make almost no sound
	ldh ($12), a   ; NR12
	ld a, $83
	ldh ($13), a   ; NR13
	ld a, $87
	ldh ($14), a   ; NR14
	ld a, $F3
	ldh ($12), a   ; Set NR12 to its expected value after starting the silent sound
	ld a, $FC
	ldh ($47), a   ; BGP

setupIO.waitForSTAT:
	; Wait till the STAT register has the right value at startup
	ldh a, ($41)
	cp BOOT_VALUE_STAT
	jr nz, setupIO.waitForSTAT

	jp handOff
