		AREA    |.text|, CODE, READONLY, ALIGN=2
        
        GLOBAL f_asm
        EXTERN matriz
		       
        THUMB
f_asm

        mov r0,#0xaa00
        mov r1,#0xbb
        orrs r1,r0
        
        ldr r0,=matriz
        ldrb r2,[r0,#0]
        ldrb r2,[r0,#6]
        
        bx lr

        END