#include "draw.h"

//To print on the screen
extern tContext sContext;

/*----------------------------------------------------------------------------
 *      Mutex
 *---------------------------------------------------------------------------*/
void draw_pixel(int x, int y)
{
	osMutexWait(mutex_display,osWaitForever);
	GrPixelDraw(&sContext, x, y);
	osMutexRelease(mutex_display);
}


/*----------------------------------------------------------------------------
 *      Draw functions
 *---------------------------------------------------------------------------*/

static void intToString(int64_t value, char * pBuf, uint32_t len, uint32_t base, uint8_t zeros) {
	static const char* pAscii = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	bool n = false;
	int pos = 0, d = 0;
	int64_t tmpValue = value;

	// the buffer must not be null and at least have a length of 2 to handle one
	// digit and null-terminator
	if (pBuf == NULL || len < 2)
			return;

	// a valid base cannot be less than 2 or larger than 36
	// a base value of 2 means binary representation. A value of 1 would mean only zeros
	// a base larger than 36 can only be used if a larger alphabet were used.
	if (base < 2 || base > 36)
			return;

	if (zeros > len)
		return;
	
	// negative value
	if (value < 0)
	{
			tmpValue = -tmpValue;
			value    = -value;
			pBuf[pos++] = '-';
			n = true;
	}

	// calculate the required length of the buffer
	do {
			pos++;
			tmpValue /= base;
	} while(tmpValue > 0);


	if (pos > len)
			// the len parameter is invalid.
			return;

	if(zeros > pos){
		pBuf[zeros] = '\0';
		do{
			pBuf[d++ + (n ? 1 : 0)] = pAscii[0]; 
		}
		while(zeros > d + pos);
	}
	else
		pBuf[pos] = '\0';

	pos += d;
	do {
			pBuf[--pos] = pAscii[value % base];
			value /= base;
	} while(value > 0);
}

void print_score() {
	int i, j;
	char pbuf[3];
	GrContextBackgroundSet(&sContext, ClrDarkGreen);
	GrContextForegroundSet(&sContext, ClrBlack);
	for(i = 97; i < 18; i++) {
		for(j = 85; j < 8; j++) {
			draw_pixel(i, j);
		}
	}
	intToString(score, pbuf, 3, 10, 0);
	GrStringDraw(&sContext, (char*)pbuf, -1, 100, 100, true);
}

void print_lives() {
	uint8_t i, j, k;
	
	// Clear screen
	GrContextForegroundSet(&sContext, ClrBlack);
	for(i = lives - 1; i < 3; i++) {
		for(j = 0; j < 4; j++) {
			for(k = 0; k < 4; k++) {
				draw_pixel(20 + (6 * i) + j, 110 + k);
			}
		}
	}
	
	// Print lives counter
	GrContextForegroundSet(&sContext, ClrDarkGreen);
	for(i = 0; i < lives - 1; i++) {
		for(j = 0; j < 4; j++) {
			for(k = 0; k < 4; k++) {
				draw_pixel(20 + (6 * i) + j, 110 + k);
			}
		}
	}
}

void clear_display_sprite(int velx, int vely, int posx, int posy) {
	int i, j;
	for(i = -2; i < 9; i++) {
		for(j = -2; j < 9; j++) {
			if(coll_mtx[i + posx][j + posy] == 0)
				GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
			else if(coll_mtx[i + posx][j + posy] == 1)
				GrContextForegroundSet(&sContext, ClrDarkBlue);
			else if(coll_mtx[i + posx][j + posy] == 2)
				GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
			else if(coll_mtx[i + posx][j + posy] == 3)
				GrContextForegroundSet(&sContext, ClrWhite);
			else if(coll_mtx[i + posx][j + posy] == 4)
				GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
			else if(coll_mtx[i + posx][j + posy] == 5)
				GrContextForegroundSet(&sContext, ClrDarkOrange);
			draw_pixel(i + posx, j + posy);
		}
	}
	return;
	
	if(velx == 1)	{
		for(i = -2; i < 9; i++) {
			for(j = -2; j < 9; j++) {
				if(coll_mtx[i + posx][j + posy] == 0)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 1)
					GrContextForegroundSet(&sContext, ClrDarkBlue);
				else if(coll_mtx[i + posx][j + posy] == 2)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 3)
					GrContextForegroundSet(&sContext, ClrWhite);
				else if(coll_mtx[i + posx][j + posy] == 4)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 5)
					GrContextForegroundSet(&sContext, ClrDarkOrange);
				draw_pixel(i + posx, j + posy);
			}
		}
	}
	
	if(velx == -1) {
		for(i = 0; i < 8; i++) {
			for(j = -1; j < 7; j++) {
				if(coll_mtx[i + posx][j + posy] == 0)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 1)
					GrContextForegroundSet(&sContext, ClrDarkBlue);
				else if(coll_mtx[i + posx][j + posy] == 2)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 3)
					GrContextForegroundSet(&sContext, ClrWhite);
				else if(coll_mtx[i + posx][j + posy] == 4)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 5)
					GrContextForegroundSet(&sContext, ClrDarkOrange);
				draw_pixel(i + posx, j + posy);
			}
		}
	}
	
	if(vely == 1) {
		for(i = -1; i < 7; i++) {
			for(j = -1; j < 7; j++) {
				if(coll_mtx[i + posx][j + posy] == 0)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 1)
					GrContextForegroundSet(&sContext, ClrDarkBlue);
				else if(coll_mtx[i + posx][j + posy] == 2)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 3)
					GrContextForegroundSet(&sContext, ClrWhite);
				else if(coll_mtx[i + posx][j + posy] == 4)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 5)
					GrContextForegroundSet(&sContext, ClrDarkOrange);
				draw_pixel(i + posx, j + posy);
			}
		}
	}
	
	if(vely == -1) {
		for(i = -1; i < 7; i++) {
			for(j = 0; j < 8; j++) {
				if(coll_mtx[i + posx][j + posy] == 0)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 1)
					GrContextForegroundSet(&sContext, ClrDarkBlue);
				else if(coll_mtx[i + posx][j + posy] == 2)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 3)
					GrContextForegroundSet(&sContext, ClrWhite);
				else if(coll_mtx[i + posx][j + posy] == 4)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 5)
					GrContextForegroundSet(&sContext, ClrDarkOrange);
				draw_pixel(i + posx, j + posy);
			}
		}
	}
	
	if(velx == 0 && vely == 0) {
		for(i = -1; i < SPRITEH; i++) {
			for(j = 0; j < SPRITEW; j++) {
				if(coll_mtx[i + posx][j + posy] == 0)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 1)
					GrContextForegroundSet(&sContext, ClrDarkBlue);
				else if(coll_mtx[i + posx][j + posy] == 2)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 3)
					GrContextForegroundSet(&sContext, ClrWhite);
				else if(coll_mtx[i + posx][j + posy] == 4)
					GrContextForegroundSet(&sContext, ClrDarkGoldenrod);
				else if(coll_mtx[i + posx][j + posy] == 5)
					GrContextForegroundSet(&sContext, ClrDarkOrange);
				draw_pixel(i + posx, j + posy);
			}
		}
	}
}

void clear_teleporting_sprite() {
	int i, j; 
	GrContextForegroundSet(&sContext, ClrDarkBlue);
	
	// baixo -> cima
	for(i = 0; i < SPRITEW + 1; i++) {
		for(j = 0; j < SPRITEH + 1; j++) {
			draw_pixel(60 + i, 87 + j);
		}
	}
	
	// cima -> baixo
	for(i = 0; i < SPRITEW + 1; i++) {
		for(j = 0; j < SPRITEH + 1; j++) {
			draw_pixel(60 + i, j);
		}
	}
}

void print_fantasmas() {
	int i, j, ghost;
	clear_display_sprite(ghost_vel_x[0], ghost_vel_y[0], ghost_pos_x[0], ghost_pos_y[0]);
	clear_display_sprite(ghost_vel_x[1], ghost_vel_y[1], ghost_pos_x[1], ghost_pos_y[1]);
	clear_display_sprite(ghost_vel_x[2], ghost_vel_y[2], ghost_pos_x[2], ghost_pos_y[2]);
	
	for(ghost = 0; ghost < 3; ghost++) {
		// Fantasma em perseguição ou fugindo
		if(ghost_state[ghost] != 2) {
			if(ghost_state[ghost] == 0) {
				// Verifica qual é o fantasma
				if(ghost == 0) GrContextForegroundSet(&sContext, ClrPink);
				else if (ghost == 1) GrContextForegroundSet(&sContext, ClrRed);
				else if (ghost == 2) GrContextForegroundSet(&sContext, ClrGreen);
			}
			else GrContextForegroundSet(&sContext, ClrSlateBlue);
			
			// Imprime o fantasma
			for(i = 0; i < SPRITEH; i++) {
				for(j = 0; j < SPRITEW; j++) {
					if(ghost_sprites[ghost_sprite][(i*7 + j) * 3 + 12] > 150) {
						draw_pixel(j + ghost_pos_x[ghost], i + ghost_pos_y[ghost]);
					}						
				}	
			}
		}
		
		else {
			GrContextForegroundSet(&sContext, ClrSlateBlue);
			// Imprime o fantasma morto
			for(i = 0; i < SPRITEH; i++) {
				for(j = 0; j < SPRITEW; j++) {
					if(eyes[(i*7 + j) * 3 + 12] > 150) {
						draw_pixel(j + ghost_pos_x[ghost], i + ghost_pos_y[ghost]);			
					}						
				}	
			}
		}
	}
	
	if(ghost_sprite == 1) ghost_sprite = 0;
	else ghost_sprite++;
}

void print_pacman() {
	int i, j;
	
	if(teleporting)
		clear_teleporting_sprite();
	else
		clear_display_sprite(pac_vel_x, pac_vel_y, pacx, pacy);
	GrContextForegroundSet(&sContext, ClrYellow);
	
	if(pacman_dead) {
		// Alexa play Despacito
		for(i = 0; i < SPRITEH; i++) {
				for(j = 0; j < SPRITEW; j++) {
					if(pacman_death_sprites[pacman_sprite][(i*SPRITEW + j) * 3 + 12] > 150)
						draw_pixel(j + pacx, i + pacy);
				}	
			}
	}
	
	else {
		if(pac_dir == 1) {
			for(i = 0; i < SPRITEH; i++) {
				for(j = 0; j < SPRITEW; j++) {
					if(pacman_sprites[pacman_sprite][(i*SPRITEW + j) * 3 + 12] > 150 &&
							(i + pacy) > 0 &&
							(i + pacy) < 95)
						draw_pixel(j + pacx, i + pacy);
				}	
			}
		}
		
		else if(pac_dir == -1) {
			for(i = 0; i < SPRITEH; i++) {
				for(j = 0; j < SPRITEW; j++) {
					if(pacman_sprites[pacman_sprite][(i*SPRITEW + j) * 3 + 12] > 150 &&
							(i + pacy) > 0 &&
							(i + pacy) < 95)
						draw_pixel(SPRITEW - j - 1 + pacx, i + pacy);
				}	
			}
		}
		
		if(pacman_sprite == 3) pacman_sprite = 0;
		else pacman_sprite++;
	}
}

void print_initial_map(){
	int i, j;
	for(i = 0; i < 128; i++) {
		for(j = 0; j < 128; j++) {
			if(mapaParaColisoes[(i*128 + j) * 3 + 15] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 16] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 17] < 50)
				GrContextForegroundSet(&sContext, ClrBlack);
			else if(mapaParaColisoes[(i*128 + j) * 3 + 15] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 16] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 17] < 50 ||
				      mapaParaColisoes[(i*128 + j) * 3 + 15] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 16] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 17] > 200)
				GrContextForegroundSet(&sContext, ClrDarkGoldenrod);		
			else if(mapaParaColisoes[(i*128 + j) * 3 + 15] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 16] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 17] < 50)
				GrContextForegroundSet(&sContext, ClrDarkGreen);	
			else if(mapaParaColisoes[(i*128 + j) * 3 + 15] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 16] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 17] > 200)
				GrContextForegroundSet(&sContext, ClrDarkBlue);	
			else if(mapaParaColisoes[(i*128 + j) * 3 + 15] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 16] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 17] < 50)
				GrContextForegroundSet(&sContext, ClrDarkGoldenrod);	
			else if(mapaParaColisoes[(i*128 + j) * 3 + 15] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 16] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 17] > 200)
				GrContextForegroundSet(&sContext, ClrWhite);
			
			draw_pixel(j, i);
		}	
	}
	
	print_pacman();
	print_score();
	print_lives();
}

void draw_big_pill() {
	uint8_t i, j;
	GrContextForegroundSet(&sContext, ClrDarkOrange);
	for(i = 0; i < 8; i++) 	{
		for(j = 0; j < 5; j++)
			draw_pixel(i + 60, j + 50);
	}
}

void erase_big_pill() {
	uint8_t i, j;
	GrContextForegroundSet(&sContext, ClrDarkBlue);
	for(i = 0; i < 8; i++)	{
		for(j = 0; j < 5; j++)
			draw_pixel(i + 60, j + 50);
	}
}
