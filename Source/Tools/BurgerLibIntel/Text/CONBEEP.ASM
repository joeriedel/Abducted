        .386
        .MODEL  FLAT,SYSCALL

        .CODE

;
; Beep the speaker using BIOS
; void ConsoleBeep(void);
;

        PUBLIC ConsoleBeep_
ConsoleBeep_ PROC USES EBX
        MOV     EAX,0E07H       ;Teletype write a BEEP char
        MOV     BL,7            ;Text color (Needed?!?)
        MOV     BH,0            ;Main page
        INT     010H            ;Call bios
        RET
        ENDP

        ENDS
        END
