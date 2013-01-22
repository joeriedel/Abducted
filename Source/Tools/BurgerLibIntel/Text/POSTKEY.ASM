        .386
        .MODEL  FLAT,SYSCALL

        .CODE

;
; Call INT 16 to post a keyboard event into the keyboard queue
; void PostKeyPress(Word ASCIIChar);
;

        PUBLIC PostKeyPress_
PostKeyPress_ PROC USES ECX
        MOV     ECX,EAX         ;Move the ASCII/Scan code
        MOV     AH,5            ;
        INT     016H            ;Perform int 10H
        AND     EAX,0FFH        ;Return 0 or 1
        RET                     ;Exit
        ENDP

        ENDS
        END
