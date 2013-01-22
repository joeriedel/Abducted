        .386
        .MODEL  FLAT,SYSCALL

	.DATA
	PUBLIC	_OutCharVector
_OutCharVector	DD OFFSET PutCharConsole_

        .CODE

	EXTRN	PutCharConsole_:PROC

;
; void CaptureText(Byte *TextPtr);
;

	PUBLIC	OutChar_
OutChar_ PROC
	JMP	[_OutCharVector]
	ENDP

        ENDS
        END
