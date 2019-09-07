
        AREA    |.text|, CODE, READONLY, ALIGN=2

		GLOBAL Zoom	
		EXTERN airplane
		EXTERN bus
		EXTERN matriz
		IMPORT limpa_tela

	THUMB                        ; Instruções do tipo Thumb-2

Zoom
;0x0000068E BD70      
	POP           {r4-r6,pc}
;0x00000690 545C      
	STRB          r4,[r3,r1]
;0x00000692 E7F4      
	B             0x0000067E
;0x00000694 E92D5FF0  
	PUSH          {r4-r12,lr}
;0x00000698 4605      
	MOV           r5,r0
;0x0000069A 4608      
	MOV           r0,r1
;0x0000069C 4616      
	MOV           r6,r2
;0x0000069E 4699      
	MOV           r9,r3
;0x000006A0 2400      
	MOVS          r4,#0x00
;0x000006A2 F7FFFFE1  
	BL.W          limpa_tela ;(0x00000668)
;0x000006A6 F8DF81D4  
	LDR.W         r8,[pc,#468]  ; @0x0000087E
;0x000006AA F8DFB1D8  
	LDR.W         r11,[pc,#472]  ; @0x00000886
;0x000006AE F108082C  
	ADD           r8,r8,#0x2C
;0x000006B2 EB050745  
	ADD           r7,r5,r5,LSL #1
0x000006B6   CMP           r9,#0x00 ;F1B90F00
0x000006BA   BEQ           0x000006D0 ;D009

0x000006BC   BL.W          limpa_tela ;(0x000005A2) ;F7FFFF71
 
0x000006C0   MOVS          r1,#0x00 ;2100
 
0x000006C2   MOVS          r0,#0x00 ;2000
 
0x000006C4   CMP           r0,#0x80 ;2880
0x000006C6   BCS           0x000007B6 ;D276
0x000006C8   CMP           r1,#0x80 ;2980
0x000006CA   BCS           0x000007B6;D274

;0x000006CC   CBZ           r6,0x00000734;B396
;Branch to 0x00000734 if R6 == 0;
			CMP R6, #0x00
			BEQ	0x00000734


0x000006CE   B             0x00000788;E05B

0x000006D0   MOVS          r0,#0x80;2080
0x000006D2   CMP           r0,r5,LSL #6;EBB01F85
0x000006D6   BLS           0x000006DC;D901

0x000006D8   BL.W          limpa_tela ;(0x000005A2)F7FFFF63

0x000006DC   MOVS          r2,#0x00;2200
0x000006DE   B             0x00000766;E042

0x000006E0   MOVS          r1,#0x00;2100
0x000006E2   B             0x0000075E;E03C

0x000006E4   CMP           r1,#0x80;2980
0x000006E6   BCS           0x0000075A;D238
0x000006E8   CMP           r2,#0x80;2A80
0x000006EA   BCS           0x0000075A;D236
;0x000006EC   CBNZ          r6,0x00000706;B95E
			 CMP	R6, #0x00
			 BNE	0x00000706
			 
0x000006EE   LDRB          r0,[r11,r4];F81B0004
0x000006F2   CMP           r0,#0xF0;28F0
0x000006F4   BCC           0x00000706;D307
0x000006F6   ADD           r0,r11,r4;EB0B0004
0x000006FA   LDRB          r3,[r0,#0x01];7843
0x000006FC   CMP           r3,#0xF0;2BF0
0x000006FE   BCC           0x00000706;D302
0x00000700   LDRB          r0,[r0,#0x02];7880
0x00000702   CMP           r0,#0xF0;28F0
0x00000704   BCS           0x0000071E;D20B
 
;0x00000706   CBZ           r6,0x00000724;B16E
			 CMP 	R6, #0x00
			 BEQ 	0x00000724
			 
0x00000708   LDR           r0,[pc,#380]  ; @0x00000888 485F
0x0000070A   LDRB          r3,[r0,r4];5D03
0x0000070C   CMP           r3,#0xF0;2BF0
0x0000070E   BCC           0x00000724;D309
0x00000710   ADD           r0,r0,r4;4420
0x00000712   LDRB          r3,[r0,#0x01];7843
0x00000714   CMP           r3,#0xF0;2BF0
0x00000716   BCC           0x00000724;D305
0x00000718   LDRB          r0,[r0,#0x02];7880
0x0000071A   CMP           r0,#0xF0;28F0
0x0000071C   BCC           0x00000724;D302
 
0x0000071E   MOV           r9,#0x00;F04F0900
0x00000722   B             0x00000728;E001
 
0x00000724   MOV           r9,#0x01;F04F0901
 
0x00000728   MOVS          r3,#0x00;2300
0x0000072A   B             0x00000756;E014
 
0x0000072C   MOVS          r0,#0x00;2000
   
0x0000072E   ADD           r12,r2,r3;EB020C03
0x00000732   B             0x00000750;E00D
0x00000734   B             0x00000770;E01C
0x00000736   ADD           r10,r1,r0;EB010A00
0x0000073A   CMP           r10,#0x80;F1BA0F80
0x0000073E   BCS           0x0000074E;D206
0x00000740   CMP           r12,#0x80;F1BC0F80
0x00000744   BCS           0x0000074E;D203
  
0x00000746   ADD           r10,r8,r10,LSL #7;EB081ACA ; ???? perdi
0x0000074A   STRB          r9,[r10,r12];F80A900C
0x0000074E   ADDS          r0,r0,#1;1C40
0x00000750   CMP           r0,r5;42A8
0x00000752   BCC           0x00000736;D3F0

0x00000754   ADDS          r3,r3,#1;1C5B
0x00000756   CMP           r3,r5;42AB
0x00000758   BCC           0x0000072C;D3E8

0x0000075A   ADDS          r4,r4,#3;1CE4

0x0000075C   ADD           r1,r1,r5;4429
0x0000075E   CMP           r1,r7,LSL #5;EBB11F47
0x00000762   BCC           0x000006E4;D3BF

0x00000764   ADD           r2,r2,r5;442A
0x00000766   CMP           r2,r5,LSL #6;EBB21F85
0x0000076A   BCC           0x000006E0;D3B9

			NOP
			ALIGN
			END