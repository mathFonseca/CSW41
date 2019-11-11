		AREA    pacman_load_images, DATA, READWRITE
		THUMB

		; Imagem do mapa
		EXPORT  mapa
        EXPORT  mapa_length
			
		EXPORT  fantasma1_imagem
        EXPORT  fantasma_length1

		EXPORT  fantasma2_imagem
        EXPORT  fantasma_length2
		
		EXPORT  fantasma3_imagem
        EXPORT  fantasma_length3

		EXPORT  fantasma4_imagem
        EXPORT  fantasma_length4

		EXPORT  pacman1
		EXPORT  pacman1_length
			
		EXPORT  pacman2
		EXPORT  pacman2_length

		EXPORT  pacman3
		EXPORT  pacman3_length		
			
		EXPORT 	pacman1_morrendo
		EXPORT 	pacman1_morrendo_length
		
		EXPORT 	pacman2_morrendo
		EXPORT 	pacman2_morrendo_length
		
		EXPORT 	pacman3_morrendo
		EXPORT 	pacman3_morrendo_length			
		
		EXPORT 	pacman4_morrendo
		EXPORT 	pacman4_morrendo_length
			
		EXPORT 	fantasma_morto
		EXPORT 	fantasma_morto_length
			
;-------------------------------------------
;Imagem do Mapa
mapa
		INCBIN pacman_imagens\Pacman_mapa_preto_branco.pgm
mapa_end
		
		ALIGN
mapa_length
        DCD     mapa_end - mapa
		
;-------------------------------------------
;Fantasma 1 
fantasma1_imagem
		INCBIN pacman_imagens\fantasma1.pgm
fantasma1_end

		ALIGN
fantasma_length1	
        DCD     fantasma1_end - fantasma1_imagem

;Fantasma2
fantasma2_imagem
		INCBIN pacman_imagens\fantasma2.pgm
fantasma2_end

		ALIGN
fantasma_length2	
        DCD     fantasma2_end - fantasma2_imagem
		
;Fantasma3
fantasma3_imagem
		INCBIN pacman_imagens\fantasma3.pgm
fantasma3_end

		ALIGN
fantasma_length3	
        DCD     fantasma3_end - fantasma3_imagem
		
;Fantasma4
fantasma4_imagem
		INCBIN pacman_imagens\fantasma4.pgm
fantasma4_end

		ALIGN
fantasma_length4	
        DCD     fantasma4_end - fantasma4_imagem

fantasma_morto
		INCBIN pacman_imagens\fantasma_olho.pgm
fantasma_morto_end

		ALIGN
fantasma_morto_length	
        DCD     fantasma_morto - fantasma_morto_end
				
;-------------------------------------------
;PACMAN - INICIO 
pacman1
		INCBIN pacman_imagens\pacman1.pgm
pacman1_end

		ALIGN
pacman1_length	
        DCD     pacman1_end - pacman1
		
; MEIO MOVIMENTACAO
pacman2
		INCBIN pacman_imagens\pacman2.pgm
pacman2_end

		ALIGN
pacman2_length	
        DCD     pacman2_end - pacman2

; FIM MOVIMENTACAO
pacman3
		INCBIN pacman_imagens\pacman3.pgm
pacman3_end

		ALIGN
pacman3_length	
        DCD     pacman3_end - pacman3
;-------------------------------------------
; PACMAN MORRENDO
pacman1_morrendo
		INCBIN pacman_imagens\pacman1_morrendo.pgm
pacman1_morrendo_end

		ALIGN
pacman1_morrendo_length	
        DCD     pacman1_morrendo_end - pacman1_morrendo
			
; PACMAN MORRENDO 2 
pacman2_morrendo
		INCBIN pacman_imagens\pacman2_morrendo.pgm
pacman2_morrendo_end

		ALIGN
pacman2_morrendo_length	
        DCD     pacman2_morrendo_end - pacman2_morrendo
			
; PACMAN MORRENDO 3 
pacman3_morrendo
		INCBIN pacman_imagens\pacman3_morrendo.pgm
pacman3_morrendo_end

		ALIGN
pacman3_morrendo_length	
        DCD     pacman3_morrendo_end - pacman3_morrendo
			
; PACMAN MORRENDO 4 
pacman4_morrendo
		INCBIN pacman_imagens\pacman4_morrendo.pgm
pacman4_morrendo_end

		ALIGN
pacman4_morrendo_length	
        DCD     pacman4_morrendo_end - pacman4_morrendo

;-------------------------------------------

		ALIGN
		END