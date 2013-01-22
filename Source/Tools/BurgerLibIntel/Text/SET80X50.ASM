        .386
        .MODEL  FLAT,SYSCALL

        .CODE

	EXTRN	FixVideoWidthHeight_:proc

;
; Set the video system to 80x50 graphics
; void Set80x50(void);
;

        PUBLIC Set80x50_
Set80x50_ PROC USES EBX
	MOV	EAX,3
	INT     010H            ;Perform int 10H
	MOV	EAX,01112H
	MOV	BL,0
	INT	010H
	CALL	FixVideoWidthHeight_
	RET                     ;Exit

        ENDP

        ENDS
        END
