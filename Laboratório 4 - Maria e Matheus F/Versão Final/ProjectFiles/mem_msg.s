		AREA    MessageText, DATA, READONLY

		EXPORT  Message1
		EXPORT 	Message1_length
		
Message1
		INCBIN mensagem1.txt
Message1_end
			
; Comprimento das imagens			
Message1_length
        DCD     Message1_end - Message1				
			
		ALIGN
		END