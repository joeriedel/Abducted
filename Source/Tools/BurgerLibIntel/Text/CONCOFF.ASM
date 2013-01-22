        .386
        .MODEL  FLAT,SYSCALL

        .CODE

;
; Deactivate the text cursor
; void ConsoleCursorOff(void);
;

        PUBLIC ConsoleCursorOff_
ConsoleCursorOff_ PROC USES ECX
        MOV     AH,1            ;Set text mode cursor size
        MOV     CH,020h         ;Shut off the cursor
        MOV     CL,0
        INT     010h            ;Call video bios
        RET
        ENDP

        ENDS
        END
