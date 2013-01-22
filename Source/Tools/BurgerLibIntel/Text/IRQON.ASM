        .386
        .MODEL  FLAT,SYSCALL

        .CODE

;
; Enable the IRQ flag (Irq's are OK!)
; void IrqOn(void);
;

        PUBLIC IrqOn_
IrqOn_  PROC
        STI                 ;Enable IRQ's
        RET
        ENDP

        ENDS
        END
