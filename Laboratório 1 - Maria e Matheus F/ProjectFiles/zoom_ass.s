		
		AREA    |.text|, CODE, READONLY, ALIGN=2
        THUMB
        GLOBAL  ZoomAssembly
		EXTERN matriz_resultado
		EXTERN airplane
		EXTERN bus
		
		
ZoomAssembly		
		;r0 <= scale
		;r1 <= image_choose
		;r2 <= zoom_choose
		PUSH {r4-r11,lr}
      
		ldr r3, =matriz_resultado
		ldr r4, =airplane
		ldr r5, =bus
		
		mov r6, #0 ;i
		mov r7, #0 ;j
		mov r10,#0 ;pixel
		mov r11,#0; pinta
		
		cmp r1, #0	; zoom aumentativo
		beq zoom_in
		cmp r1,#1	; zoom diminutivo
		beq zoom_out
		b retorno
		
zoom_in
		
; --- FOR i < 64 * scale
for_i
; --- FOR j < 96 * scale
for_j
; if j < 128 && i < 128
		CMP R7, #128	; j < 128?
		BCS soma_pixel 	; Se não, ja pula pro fim do if.
						; Se sim, continue
		CMP	R6, #128	; i < 128?
		BCS soma_pixel	; Se não, ja pula pro fim do if.
						; Se sim, continue.
; if image_choose true or false.
		CMP R1, #0		; 
		BEQ if_airplane	; 0 = airplane	
		BNE if_bus		; 1 = bus
		
if_airplane
; if posição >= 0xF0
		PUSH {R1}
		LDRB R1,[R4,R10]	; Lê airplane[pixel] e salva em R3
		CMP R1, #0xF0		; Se valor >= 0xF0, branco.
		BCS pinta_branco
		B pinta_preto		; Se não, preto.
if_bus
; if posição >= 0xF0	
		LDRB R1,[R5,R10]	; Lê bus[pixel]. NÃO POSSO SALVAR EM R3 AAAAAA.
		CMP R1, #0xF0		; Se valor >= 0xF0, branco.
		BCS pinta_branco
		B pinta_preto		; Se não, preto.
pinta_branco
		MOV R11, #0
		B cor_definida
pinta_preto
		MOV R11, #1
		B cor_definida

cor_definida
		B preenche_matriz

preenche_matriz				;r6 = i || r7 = j || r8 = k || r9 = l
		MOV R8, R6			; k = i
		PUSH {R6}			; guarda i na pilha para depois.
		PUSH {R10}			; guarda pixel na pilha para depois.
		PUSH {R5}			; guarda bus, afinal, tamo em bus mesmo
		MOV R5, #0
for_k
		ADD R5, R6, R0		; y = i + scale
		CMP R8, R5			; k < i + scale?
		BCS acabou			; Se sim, termina
							; Se não, continua
		MOV R9, R7 			; l = j
for_l
		ADD R10, R7, R0		; x = j + scale
		CMP R9, R10			; l < j + scale?
		BCS fim_for_l		; Se sim, termina.
							; Se não, continua
if_interno_loop_k_l
		CMP R9, #128		; l < 128?
		BCS	if_false		; Se não, sai do if
							; Se sim, continua
		CMP R8, #128		; k < 128?
		BCS	if_false		; Se não, sai do if
							; Se sim, continua
		; matriz_resultados[l][k] = pinta
		MOV R12, #96		
		MLA R12, R12, R8, R9; R12 = K + (96 * L)
		STRB R11, [R3, R12]
		B if_false			; Após preencher, executa o l++ naturalmente.
if_false
		ADD R9, R9, #1		; l++
		B for_l
fim_for_l
		ADD R8, R8, #1		; k ++
		B for_k
acabou	
		POP {R5}
		POP {R10}			; Retorna pixel original 
		POP {R6}			; Retorna i original.
		B soma_pixel		
		; for do k e do l
		; pixel += 3
soma_pixel
		POP {R1}
		ADD R10, R10, #3
; --- FIM FOR j < 96*scale				
		ADD R7, R7, R0 		; j+=scale
		MOV	R12, #96	
		MUL R12, R12, R0	; 96 * scale
		CMP R7, R12			; j < 96 * scale?
		BLO for_j			; if YES, continue
		B fim_for_j			; if NOT, fim for j.
fim_for_j
		MOV R7, #0			; j = 0.
		ADD R6, R6, R0 		; i+=scale
		MOV	R12, #64
		MUL R12, R12, R0	; 64 * scale
		CMP R6, R12			; i < 64 * scale?
		BLO for_i			; if YES, continue
		B retorno			; if NOT. fim for i
; fim_for_i significa que ele entrou no IF inicial para Zoom In, então encerramos a execução dessa função.
; --- FIM FOR i < 64*scale 		
zoom_out
for_i_out
	; Executa for j várias vezes
for_j_out
	; Executa conteudo do for j 96/scale vezes.
if_externo_out
	CMP R6, #128		; i < 128?
	BCS fim_if_externo_out
	
	CMP R7, #128		; j < 128?
	BCS fim_if_externo_out
	B if_imagem_out

if_imagem_out
	CMP R2, #0
	BEQ	airplane_out
	B	bus_out
	
airplane_out
	PUSH {R1}
	LDRB R1,[R4,R10]	; Lê airplane[pixel] e salva em R1
	CMP R1, #0xF0		; Se valor >= 0xF0, branco.
	BCS pinta_branco_out
	B pinta_preto_out		; Se não, preto.	
bus_out
	PUSH {R1}
	LDRB R1,[R5,R10]	; Lê bus[pixel] e salva em R1
	CMP R1, #0xF0		; Se valor >= 0xF0, branco.
	BCS pinta_branco_out
	B pinta_preto_out		; Se não, preto.
	
pinta_branco_out
	MOV R11, #0
	B pinta_matriz
	
pinta_preto_out
	MOV R11, #1
	B pinta_matriz
	
pinta_matriz
	PUSH {R9, R10, R12}
	MOV R9, R6
	MOV R10, R7
	UDIV R9, R9, R0		; i/scale
	UDIV R10, R10, R0	; j/scale
	MOV R12, #96
	MLA	R12, R10, R12, R9; ((j/scale * 96) + i/scale) 
	STRB R11, [R3, R12]
fim_if_externo_out
	PUSH {R1}
	MOV R1, #3
	MLA R10, R0, R1, R10;´pixel = pixel + (3*scale)
	POP {R1}
	ADD R7, R7, R0		; j += scale
	CMP R7, #96			; j < 96?
	BLO for_j_out			; Se sim, continua
	B soma_pixel_out	; Se não, termina
soma_pixel_out
	PUSH {R11, R7}
	MOV R11, #3
	MOV R7, R10			
	MUL R11, R11, R0	; 96 * 3
	SUB	R7, R0, #1		; scale - 1;
	MLA R10, R7, R11, R10; pixel = pixel + ( (96*3) * (scale -1))
	POP {R7, R11}
fim_for_j_out
	MOV R7, #0			; j = 0
	ADD R6, R6, R0		; i += scale
	CMP R6, #64			; i < 64?
	BLO for_i_out			; Se sim, continua
	B fim_zoom_out		; Se não, termina
fim_zoom_out
		B retorno
retorno
		POP {r11,r10,r9,r8,r7,r6,r5,r4}
		BX LR

		NOP
		ALIGN
        END