		AREA    |.text|, CODE, READONLY, ALIGN=2
        
        GLOBAL f_asm
        EXTERN airplane
		EXTERN imagem
		       
        THUMB
f_asm
		;R0->aumenta=0 e diminui=1
		;R1->escala
        LDR R2,=airplane
		LDR R3,=imagem
		CBZ	R0,	aumenta ;salta se for igal a 0
		CBNZ diminui	;salta se nao for igual a 0

aumenta	
		MOV R4, #0 ;pixel
		
linha1

coluna
		
		
		
diminui


		
		
		
termina
        
        bx lr

        END