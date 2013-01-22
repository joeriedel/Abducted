        .386
        .MODEL  FLAT,SYSCALL

        .CODE

;
; Disable the IRQ flag (Irq's are OFF!)
; void IrqOff(void);
;

        PUBLIC IrqOff_
IrqOff_ PROC
        CLI                 ;Disable IRQ's
        RET
        ENDP

        ENDS
        END
