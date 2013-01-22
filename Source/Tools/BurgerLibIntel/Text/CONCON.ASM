        .386
        .MODEL  FLAT,SYSCALL

        .CODE

;
; Activate the text cursor
; void ConsoleCursorOn(void);
;

        PUBLIC ConsoleCursorOn_
ConsoleCursorOn_ PROC USES ECX
        MOV     AH,1            ;Set text mode cursor size
        MOV     CH,00Dh         ;Turn on the cursor
        MOV     CL,00Eh
        INT     010h            ;Call the video bios
        RET
        ENDP

        ENDS
        END
