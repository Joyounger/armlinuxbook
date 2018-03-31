;
; Copyright (c) 2004-2005, Nokia
; All rights reserved.
;
; Redistribution and use in source and binary forms, with or without
; modification, are permitted provided that the following conditions are met:
;
; Redistributions of source code must retain the above copyright notice, this
; list of conditions and the following disclaimer. 
;
; Redistributions in binary form must reproduce the above copyright notice,
; this list of conditions and the following disclaimer in the documentation
; and/or other materials provided with the distribution. 
;
; Neither the name of Nokia nor the names of its contributors may be used to
; endorse or promote products derived from this software without specific
; prior written permission. 
;
; THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
; IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
; ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
; LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
; CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
; SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
; INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
; CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
; ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
; POSSIBILITY OF SUCH DAMAGE.
;
; Toshihiro Kobayashi <toshihiro.kobayashi@nokia.com>
; 2004/07/13:  DSP Gateway version 3.2
;

	.bss xsp_save, 2
	.bss ssp_save, 1

	.text
	.def ___suspend
	.def ___resume
___suspend:
	AADD #-3, SP

	; save RETA
	MOV RETA, dbl(@#00h)

	; save stack pointers
	MOV XSP, AC0
	MOV AC0, dbl(*(#xsp_save))
	MOV mmap(SSP), AC0
	MOV AC0, *(#ssp_save)

$1	B $1	; infinite loop
	;RESET

___resume:
	; restore stack pointers
	MOV dbl(*(#xsp_save)), AC0
	MOV AC0, XSP
	MOV *(#ssp_save), AC0
	MOV AC0, mmap(SSP)

	; processor initialization
	AND #0f91fh, mmap(ST1_55)
	OR #04100h, mmap(ST1_55)
	AND #0fa00h, mmap(ST2_55)
	OR #08000h, mmap(ST2_55)
	BCLR ST3_SATA
	BCLR ST3_SMUL

	; restore RETA
	MOV dbl(@#00h), RETA

	AADD #3, SP
	RET
