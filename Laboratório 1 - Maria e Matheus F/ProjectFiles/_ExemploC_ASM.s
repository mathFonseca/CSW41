
/*		AREA    |.text|, CODE, READONLY, ALIGN=2
 
        GLOBAL f_zoom_assembly
        EXTERN matriz
		IMPORT limpaMatriz
 
        THUMB
;void Zoom (uint8_t scale, uint8_t cor, uint8_t image_choose, uint8_t zoom_choose)
;				R0,				R1,			R2,				R3
f_zoom_assembly
; 64 = R4
; 96 = R5
; i = R6
; j = R7
; k = R8
; l = R9
; pixel = R10
; pinta = R11

; Push em R0 para não perder seu valor.
		PUSH{R0}
; Move R1 para R0, enviando R0 como parâmetro para limpaMatriz		
		MOV R0, R1
		PUSH {LR}
		BL limpaMatriz	; Vai levar R0 como parâmetro.
		POP {LR}
; Retorna valor original de R0.
		POP {R0}
; Bloco IF THEN para decidir que tipo de zoom é
		CMP R3, #0	; 0 = False = Zoom In. (Aumentar)
		ITE	EQ
			BLEQ	zoom_in
			BLNE	zoom_out
 zoom_in
 ; Limpa a tela se R4 = scale(R0)*64 < 128
 ; Não alterar R4 e R5.
		MOV R4, #64
		MUL R4, R0, R4
		CMP R4, #128
		IT	LO
			BLO	limpa_tela
		MOV R5, #96
		MUL R5, R0, R5
		MOV R6, #0 ; nosso i
		MOV R7, #0 ; nosso j		
for_zoom_in_i
		CMP	R6, R4 
		BHS	fim_for_zoom_in_i
		BL for_zoom_in_j
for_zoom_in_j
		CMP	R7, R5
		BHS fim_for_zoom_in_j
; Checa se i e j já passaram de 128.		
		CMP R6, #128
		BHS soma_pixel
		; sequencia de IFS
		;if((image_choose==false && (airplane[pixel] >= 0xF0 && airplane[pixel + 1] >= 0xF0 && airplane[pixel + 2] >= 0xF0)) ||
		;					 (image_choose==true && (bus[pixel] >= 0xF0 && bus[pixel + 1] >= 0xF0 && bus[pixel + 2] >= 0xF0)) )
		;					pinta=0;
		;				else
		;					pinta=1;	
		MOV R8, #0 ; nosso k
		MOV R9, #0 ; nosso l
for_zoom_in_k
		CMP R8, R0
		BHS soma_pixel
for_zoom_in_l
		CMP R9, R0
		BHS fim_for_zoom_in_l
		;if( (j+l) < 128 && (i+k) < 128)
		;matriz_resultado[j+l][i+k] = pinta;
fim_for_zoom_in_l
fim_zoom_in_k
soma_pixel
		; soma pixel
		ADD R10, R10, #3
		ADD R7, R7, R0
fim_for_zoom_in_j
		ADD R6, R6, R0
fim_for_zoom_in_i
zoom_out
        BX LR
 
        END*/