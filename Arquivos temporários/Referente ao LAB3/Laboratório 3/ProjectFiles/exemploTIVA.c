/*============================================================================
 *                    Exemplos de utilização do Kit
 *              EK-TM4C1294XL + Educational BooterPack MKII 
 *---------------------------------------------------------------------------*
 *                    Prof. André Schneider de Oliveira
 *            Universidade Tecnológica Federal do Paraná (UTFPR)
 *===========================================================================
 * Autores das bibliotecas:
 * 		Allan Patrick de Souza - <allansouza@alunos.utfpr.edu.br>
 * 		Guilherme Jacichen     - <jacichen@alunos.utfpr.edu.br>
 * 		Jessica Isoton Sampaio - <jessicasampaio@alunos.utfpr.edu.br>
 * 		Mariana Carrião        - <mcarriao@alunos.utfpr.edu.br>
 *===========================================================================*/
#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"

/*----------------------------------------------------------------------------
 * include libraries from drivers
 *----------------------------------------------------------------------------*/
#include "cfaf128x128x16.h"
#include "buttons.h"
#include "buzzer.h"
#include "joy.h"
#include "draw.h"

//To print on the screen
tContext sContext;

// Timer
osTimerId timer1;

//Mutex
osMutexId mutex_display;
osMutexDef(mutex_display);

// Thread IDs
osThreadId thread_int_usuario_id, thread_pacman_id, thread_fantasmas_id, thread_painel_id, thread_pilulas_id;

// Pacman
int pacx = PAC_INITIAL_X, pacy = PAC_INITIAL_Y;
int pac_dir = 1; // 1 = direita, -1 = esquerda
int pac_vel_x = 0, pac_vel_y = 0;
int pacman_sprite = 0;
bool pacman_dead = false;
bool teleporting = false;
const unsigned char* pacman_sprites[] = {pacman0, pacman1, pacman2, pacman1};
const unsigned char* pacman_death_sprites[] = {deadPacman0, deadPacman1, deadPacman2, deadPacman3, deadPacman4, deadPacman5, deadPacman6};

// Ghosts
uint8_t ghost_pos_x[] = {GHOST_INITIAL_X, GHOST_INITIAL_X, GHOST_INITIAL_X};
uint8_t ghost_pos_y[] = {GHOST_INITIAL_Y, GHOST_INITIAL_Y, GHOST_INITIAL_Y};
uint8_t ghost_vel_x[] = {0, 0, 0};
uint8_t ghost_vel_y[] = {0, 0, 0};
uint8_t ghost_last_move[] = {0, 0, 0}; // 1 se foi horizontal
uint8_t ghost_leaving_spawn[] = {8, 8, 8};
uint32_t startTick_ghost_escape = 0;
int ghost_sprite = 0;
int ghost_moves = 0;
const unsigned char* ghost_sprites[] = {ghost0, ghost1};
int ghost_state[] = {0, 0, 0}; // 0 = normal, 1 = fugindo, 2 = morto

// Pills
bool big_pill_enabled = false;
uint32_t startTick_big_pill;
int pill_counter = 116;

// Collision matrix
uint8_t coll_mtx[128][128];

// Player input
int8_t in_x = 0, in_y = 0;

// Score and lives
int score = 0;
uint8_t lives = 4;

// Sounds
uint16_t buzzer_vol = 0xFFFF;
uint16_t buzzer_per = 3;

// Pacman acabou de tocar numa vitamina?
bool touchedVitamin = false;

/*----------------------------------------------------------------------------
 *    Initializations
 *---------------------------------------------------------------------------*/

void init_all(){
	cfaf128x128x16Init();
	joy_init();
	buzzer_init(); 
	button_init();
	buzzer_vol_set(buzzer_vol);
	buzzer_write(false);
}

void init_display_context(){
	uint8_t i;
	GrContextInit(&sContext, &g_sCfaf128x128x16);
	
	GrFlush(&sContext);
	GrContextFontSet(&sContext, g_psFontFixed6x8);
	
	GrContextForegroundSet(&sContext, ClrYellow);
	GrContextBackgroundSet(&sContext, ClrBlack);
}

void init_coll_mtx() {
	uint8_t i, j;
	for(i = 0; i < 128; i++) {
		for(j = 0; j < 128; j++) {
			// 0: Parede
			// 1: Lugar vazio
			// 2: Pílula
			// 3: Vitamina
			// 4: Saída do spawn de fantasmas
			if(mapaParaColisoes[(i*128 + j) * 3 + 15] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 16] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 17] > 200)
				coll_mtx[j][i] = 1;
			else if(mapaParaColisoes[(i*128 + j) * 3 + 15] > 180 && mapaParaColisoes[(i*128 + j) * 3 + 16] < 80 && mapaParaColisoes[(i*128 + j) * 3 + 17] < 80)
				coll_mtx[j][i] = 2;
			else if(mapaParaColisoes[(i*128 + j) * 3 + 15] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 16] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 17] > 200)
				coll_mtx[j][i] = 3;
			else if(mapaParaColisoes[(i*128 + j) * 3 + 15] > 200 && mapaParaColisoes[(i*128 + j) * 3 + 16] < 50 && mapaParaColisoes[(i*128 + j) * 3 + 17] > 200)
				coll_mtx[j][i] = 4;
			else
				coll_mtx[j][i] = 0;
		}
	}
}

/*----------------------------------------------------------------------------
 *  Utilities
 *---------------------------------------------------------------------------*/
bool button_read_debounce_s1() {
	uint8_t i = 0;
	while(i < 50) {
		if(button_read_s1())
			i++;
		else
			return false;
	}
	return true;
}

void reset_game() {
	buzzer_write(false);
	
	osTimerStop(timer1);
	lives = 4;
	pill_counter = 116;
	
	pac_vel_x = 0;
	pac_vel_y = 0;
	pacx = PAC_INITIAL_X;
	pacy = PAC_INITIAL_Y;
	pac_dir = 1;
	pacman_dead = false;
	teleporting = false;
	score = 0;
	ghost_pos_x[0] = ghost_pos_x[1] = ghost_pos_x[2] = GHOST_INITIAL_X;
	ghost_pos_y[0] = ghost_pos_y[1] = ghost_pos_y[2] = GHOST_INITIAL_Y;
	ghost_vel_x[0] = ghost_vel_x[1] = ghost_vel_x[2] = 0;
	ghost_vel_y[0] = ghost_vel_y[1] = ghost_vel_y[2] = 0;
	ghost_state[0] = ghost_state[1] = ghost_state[2] = 0;
	ghost_leaving_spawn[0] = ghost_leaving_spawn[1] = ghost_leaving_spawn[2] = 8;
	
	in_x = 0;
	in_y = 0;
	
	touchedVitamin = false;	
	
	// Initalize collision matrix
	init_coll_mtx();
	
	// Draw static elements (map, pills, scoreboard)
	print_initial_map();
	print_score();
	
	return;	
}

void kill_pacman() {
	buzzer_per = 6;
	
	lives--;
	pac_dir = 1;
	pacman_dead = true;
	pacman_sprite = 0;
	teleporting = false;

	ghost_vel_x[0] = ghost_vel_x[1] = ghost_vel_x[2] = 0;
	ghost_vel_y[0] = ghost_vel_y[1] = ghost_vel_y[2] = 0;
	ghost_state[0] = ghost_state[1] = ghost_state[2] = 0;
	ghost_leaving_spawn[0] = ghost_leaving_spawn[1] = ghost_leaving_spawn[2] = 8;
	in_x = 0;
	in_y = 0;
	touchedVitamin = false;
}

void reset_positions() {
	buzzer_write(false);
	
	clear_display_sprite(pac_vel_x, pac_vel_y, pacx, pacy);
	clear_display_sprite(ghost_vel_x[0], ghost_vel_y[0], ghost_pos_x[0], ghost_pos_y[0]);
	clear_display_sprite(ghost_vel_x[1], ghost_vel_y[1], ghost_pos_x[1], ghost_pos_y[1]);
	clear_display_sprite(ghost_vel_x[2], ghost_vel_y[2], ghost_pos_x[2], ghost_pos_y[2]);
	
	pac_vel_x = 0;
	pac_vel_y = 0;
	pacx = PAC_INITIAL_X;
	pacy = PAC_INITIAL_Y;
	pacman_dead = false;
	pacman_sprite = 0;
	teleporting = false;
	
	ghost_pos_x[0] = ghost_pos_x[1] = ghost_pos_x[2] = GHOST_INITIAL_X;
	ghost_pos_y[0] = ghost_pos_y[1] = ghost_pos_y[2] = GHOST_INITIAL_Y;
}

//verdadeiro se pode andar
bool moveCheckPacMan() {
	int i = 0, j = 0; 
	if(pac_vel_x == 1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[pacx+7][pacy+i] == 0 || coll_mtx[pacx+7][pacy+i] == 4)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(pac_vel_x == -1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[pacx-1][pacy+i] == 0 || coll_mtx[pacx+7][pacy+i] == 4)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(pac_vel_y == 1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[pacx+i][pacy+8] == 0 || coll_mtx[pacx+7][pacy+i] == 4)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(pac_vel_y == -1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[pacx+i][pacy-2] == 0 || coll_mtx[pacx+7][pacy+i] == 4)
				return false;
		}
		//nao encontrou parede
		return true;
	}
}

bool possibleMoveCheckPacMan() {
	int i = 0, j = 0; 
	if(in_x == 1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[pacx+7][pacy+i] == 0 || coll_mtx[pacx+7][pacy+i] == 4)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(in_x == -1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[pacx-1][pacy+i] == 0 || coll_mtx[pacx+7][pacy+i] == 4)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(in_y == 1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[pacx+i][pacy+8] == 0 || coll_mtx[pacx+7][pacy+i] == 4)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(in_y == -1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[pacx+i][pacy-2] == 0 || coll_mtx[pacx+7][pacy+i] == 4)
				return false;
		}
		//nao encontrou parede
		return true;
	}
}

bool moveCheckGhost(int posx, int posy, int velx, int vely) {
	int i = 0, j = 0; 
	if(velx == 1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[posx+7][posy+i] == 0)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(velx == -1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[posx-1][posy+i] == 0)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(vely == 1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[posx+i][posy+7] == 0)
				return false;
		}
		//nao encontrou parede
		return true;
	}
	else if(vely == -1)
	{
		for(i = 0; i < 7; i++)
		{
			if(coll_mtx[posx+i][posy-1] == 0)
				return false;
		}
		//nao encontrou parede
		return true;
	}
}

int collision_ghosts(){
  if((pacx >= ghost_pos_x[0] && pacx <= (ghost_pos_x[0] + SPRITEW)) ||
      ((pacx + SPRITEW) >= ghost_pos_x[0] && (pacx + SPRITEW) <= (ghost_pos_x[0] + SPRITEW)) )
	{
    if((pacy >= ghost_pos_y[0] && pacy <= (ghost_pos_y[0] + SPRITEH)) ||
        ((pacy + SPRITEH) >= ghost_pos_y[0] && (pacy + SPRITEH) <= (ghost_pos_y[0] + SPRITEH)) )
		{
			if(ghost_state[0]!= 2)
				return 0;
    }
  }
	
	if((pacx >= ghost_pos_x[1] && pacx <= (ghost_pos_x[1] + SPRITEW)) ||
      ((pacx + SPRITEW) >= ghost_pos_x[1] && (pacx + SPRITEW) <= (ghost_pos_x[1] + SPRITEW)) )
	{
    if((pacy >= ghost_pos_y[1] && pacy <= (ghost_pos_y[1] + SPRITEH)) ||
        ((pacy + SPRITEH) >= ghost_pos_y[1] && (pacy + SPRITEH) <= (ghost_pos_y[1] + SPRITEH)) )
		{
			if(ghost_state[1]!= 2)
				return 1;
    }
  }
	
	if((pacx >= ghost_pos_x[2] && pacx <= (ghost_pos_x[2] + SPRITEW)) ||
      ((pacx + SPRITEW) >= ghost_pos_x[2] && (pacx + SPRITEW) <= (ghost_pos_x[2] + SPRITEW)) )
	{
    if((pacy >= ghost_pos_y[2] && pacy <= (ghost_pos_y[2] + SPRITEH)) ||
        ((pacy + SPRITEH) >= ghost_pos_y[2] && (pacy + SPRITEH) <= (ghost_pos_y[2] + SPRITEH)) )
		{
			if(ghost_state[2]!= 2)
				return 2;
    }
  }
	
  return 4;
}

/*----------------------------------------------------------------------------
 *      Threads
 *---------------------------------------------------------------------------*/

void thread_int_usuario(void const *argument){
	while(1) {
		osSignalWait(0x01,osWaitForever);
		
		// X axis movement
		if(joy_read_x() > 3000)
			in_x = 1;
		else if(joy_read_x() < 1000)
			in_x = -1;
		else
			in_x = 0;
		
		// Y axis movement
		if(joy_read_y() > 3000)
			in_y = -1;
		else if(joy_read_y() < 1000)
			in_y = 1;
		else
			in_y = 0;
		
		// Reset button
		if(button_read_debounce_s1()) {
			reset_game();
			osTimerStart(timer1, TIMER_PERIOD);
		}
		
		// Envia signal p/ o pacman e fantasmas
		osSignalSet(thread_pacman_id,0x01);
		
	}
}
osThreadDef(thread_int_usuario, osPriorityNormal, 1, 0);

void thread_fantasmas(void const *argument) {
	int random_number;
	int cont_dead_ghosts;
	int i, j, ghost, random_cap;
	int collided;
	
	while(1)
	{
		osSignalWait(0x01, osWaitForever);
		
		random_number = 0;
		cont_dead_ghosts = 0;
		i = 0;
		
		if(touchedVitamin == true) {
			ghost_state[0] = ghost_state[1] = ghost_state[2] = 1;
			startTick_ghost_escape = osKernelSysTick();
			touchedVitamin = false;
		}
		else if(startTick_ghost_escape != 0 && (((osKernelSysTick() - startTick_ghost_escape) / (float)osKernelSysTickFrequency) * 145.0) > 8000)
		{
			startTick_ghost_escape=0;
			for(i=0; i < 3; i++)
			{
				if(ghost_state[i] != 0)
					ghost_state[i] = 0;
			}
		}
		
		for(i = 0; i < 3; i++)
		{
			if(ghost_state[i]!=0 && ghost_pos_x[i] == GHOST_RET_POINT_X && ghost_pos_y[i] == GHOST_RET_POINT_Y)
				ghost_state[i] = 0;
		}
		
		if(!pacman_dead && ghost_moves > 0) {	
			for(ghost = 0; ghost < 3; ghost++) {
				if(ghost_state[ghost] == 0) {
					random_number = rand()%100;
					if(ghost == 0) random_cap = 50;
					if(ghost == 1) random_cap = 30;
					if(ghost == 2) random_cap = 15;
					
					if(ghost_leaving_spawn[ghost] > 0) {
						ghost_vel_y[ghost] = -1;
						ghost_leaving_spawn[ghost]--;
						if(ghost_leaving_spawn[2] == 0) {
							for(i = 60; i < 68; i++)
								for(j = 35; j < 37; j++)
									coll_mtx[i][j] = 0;
						}
					}
					
					else if(random_number > random_cap) {
						if(ghost_pos_x[ghost] < pacx && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],1,0) &&
								ghost_last_move[ghost] == 0)
						{
							ghost_last_move[ghost] = 1;
							ghost_vel_x[ghost] = 1;
							ghost_vel_y[ghost] = 0;
						}
						else if(ghost_pos_x[ghost] > pacx && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],-1,0) &&
								ghost_last_move[ghost] == 0)
						{
							ghost_last_move[ghost] = 1;
							ghost_vel_x[ghost] = -1;
							ghost_vel_y[ghost] = 0;
						}
						else if(ghost_pos_y[ghost] < pacy && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,1) &&
								ghost_last_move[ghost] == 1)
						{
							ghost_last_move[ghost] = 0;
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = 1;
						}
						else if(ghost_pos_y[ghost] > pacy && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,-1) &&
								ghost_last_move[ghost] == 1)
						{
							ghost_last_move[ghost] = 0;
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = -1;
						}
						
						else if(ghost_pos_x[ghost] < pacx && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],1,0))
						{
							ghost_last_move[ghost] = 1;
							ghost_vel_x[ghost] = 1;
							ghost_vel_y[ghost] = 0;
						}
						else if(ghost_pos_x[ghost] > pacx && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],-1,0))
						{
							ghost_last_move[ghost] = 1;
							ghost_vel_x[ghost] = -1;
							ghost_vel_y[ghost] = 0;
						}
						else if(ghost_pos_y[ghost] < pacy && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,1))
						{
							ghost_last_move[ghost] = 0;
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = 1;
						}
						else if(ghost_pos_y[ghost] > pacy && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,-1))
						{
							ghost_last_move[ghost] = 0;
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = -1;
						}
						
						else if(moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],1,0) &&
								ghost_last_move[ghost] == 1)
						{
							ghost_last_move[ghost] = 1;
							ghost_vel_x[ghost] = 1;
							ghost_vel_y[ghost] = 0;
						}
						else if(moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],-1,0) &&
								ghost_last_move[ghost] == 1)
						{
							ghost_last_move[ghost] = 1;
							ghost_vel_x[ghost] = -1;
							ghost_vel_y[ghost] = 0;
						}
						else if(moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,1) &&
								ghost_last_move[ghost] == 0)
						{
							ghost_last_move[ghost] = 0;
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = 1;
						}
						else if(moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,-1) &&
								ghost_last_move[ghost] == 0)
						{
							ghost_last_move[ghost] = 0;
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = -1;
						}					
						
						else {
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = 0;
						}
					}
					else {
						if(moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,-1))
						{
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = -1;
						}
						else if(moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,1))
						{
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = 1;
						}
						else if(moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],-1,0))
						{
							ghost_vel_x[ghost] = -1;
							ghost_vel_y[ghost] = 0;
						}
						else if(moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],1,0))
						{
							ghost_vel_x[ghost] = 1;
							ghost_vel_y[ghost] = 0;
						}
						else
						{
							ghost_vel_x[ghost] = 0;
							ghost_vel_y[ghost] = 0;
						}
					}
				}
				else
				{
					if(ghost_pos_x[ghost] < GHOST_RET_POINT_X && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],1,0))
					{
						ghost_vel_x[ghost] = 1;
						ghost_vel_y[ghost] = 0;
					}
					else if(ghost_pos_x[ghost] > GHOST_RET_POINT_X && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],-1,0))
					{
						ghost_vel_x[ghost] = -1;
						ghost_vel_y[ghost] = 0;
					}
					else if(ghost_pos_y[ghost] < GHOST_RET_POINT_Y && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,1))
					{
						ghost_vel_x[ghost] = 0;
						ghost_vel_y[ghost] = 1;
					}
					else if(ghost_pos_y[ghost] > GHOST_RET_POINT_Y && moveCheckGhost(ghost_pos_x[ghost],ghost_pos_y[ghost],0,-1))
					{
						ghost_vel_x[ghost] = 0;
						ghost_vel_y[ghost] = -1;
					}
					else
					{
						ghost_vel_x[ghost] = 0;
						ghost_vel_y[ghost] = 0;
					}
				}
			}
			
			ghost_pos_x[0] += ghost_vel_x[0];
			ghost_pos_y[0] += ghost_vel_y[0];
			
			ghost_pos_x[1] += ghost_vel_x[1];
			ghost_pos_y[1] += ghost_vel_y[1];
			
			ghost_pos_x[2] += ghost_vel_x[2];
			ghost_pos_y[2] += ghost_vel_y[2];
			
			collided = collision_ghosts();
			if(collided != 4) {
				if(ghost_state[collided]==1) {
					ghost_state[collided] = 2;
					for(i = 0; i < 3; i++)
					{
						if(ghost_state[i] == 2)
							cont_dead_ghosts++;
					}
					score+=20*2^(cont_dead_ghosts-1);
				}
				
				else if(ghost_state[collided]==0) 
					kill_pacman();
			}
		}
		
		if(ghost_moves == 2)
			ghost_moves = 0;
		else
			ghost_moves++;
		
		osSignalSet(thread_painel_id,0x01);
	}
}
osThreadDef(thread_fantasmas, osPriorityNormal, 1, 0);

void thread_pacman(void const *argument){
	while(1) {
		// Aguarda flag p/ acordar
		// SF0: user input
		osSignalWait(0x01, osWaitForever);
		
		if(!pacman_dead) {
			if(teleporting) {
				// baixo -> cima
				if(pac_vel_y == 1) {
					if(pacy == 4)
						teleporting = false;
					else if(pacy < 95)
						pacy++;
					else
						pacy = -7;
				}
				
				// cima -> baixo
				else if(pac_vel_y == -1) {
					if(pacy == 85)
						teleporting = false;
					else if(pacy > -7 && pacy > 85)
						pacy--;
					else
						pacy = 95;
				}
			}
			
			// Teleportando cima -> baixo
			else if((pacx == 60 || pacx == 61) && pacy == 1 && pac_vel_y == -1)
				teleporting = true;
			
			// Teleportando baixo -> cima
			else if((pacx == 60 || pacx == 61) && pacy == 87 && pac_vel_y == 1)
				teleporting = true;
			
			else {
				// Processa input do usuário
				if(in_x == 1 && pac_vel_x != 1 && possibleMoveCheckPacMan())
				{
					pac_vel_x = 1;
					pac_vel_y = 0;
					pac_dir = 1;
				}
				if(in_x == -1 && pac_vel_x != -1 && possibleMoveCheckPacMan())
				{
					pac_vel_x = -1;
					pac_vel_y = 0;
					pac_dir = -1;
				}
				
				if(in_y == 1 && pac_vel_y != 1 && possibleMoveCheckPacMan())
				{
					pac_vel_x = 0;
					pac_vel_y = 1;
				}
				if(in_y == -1 && pac_vel_y != -1 && possibleMoveCheckPacMan())
				{
					pac_vel_x = 0;
					pac_vel_y = -1;
				}
				
				// Realiza a movimentação
				if(pac_vel_x == 1) {
					if(moveCheckPacMan())
						pacx += 1;
					else
						pac_vel_x = 0;
				}
				else if(pac_vel_x == -1) {
					if(moveCheckPacMan())
						pacx -= 1;
					else
						pac_vel_x = 0;
				}
				
				else if(pac_vel_y == 1) {
					if(moveCheckPacMan())
						pacy += 1;
					else
						pac_vel_y = 0;
				}
				else if(pac_vel_y == -1) {
					if(moveCheckPacMan())
						pacy -= 1;
					else
						pac_vel_y = 0;
				}
			}
			
			if(startTick_ghost_escape != 0) {
				buzzer_per--;
				if(buzzer_per < 3)
						buzzer_per = 8;
				buzzer_per_set(buzzer_per);
				buzzer_write(true);
			}
		}
		
		else {
			buzzer_per_set(buzzer_per);
			buzzer_per++;
			buzzer_write(true);
			
			pacman_sprite++;
			if(pacman_sprite == 6)
				reset_positions();
		}
		
		osSignalSet(thread_pilulas_id,0x01);
	}
}
osThreadDef(thread_pacman, osPriorityNormal, 1, 0);

void thread_painel(void const *argument){
	while(1) {
		osSignalWait(0x03, osWaitForever);
		print_fantasmas();
		print_pacman();
		if(pacman_dead) print_lives();
		if((lives == 0 || pill_counter == 0) && !pacman_dead) {
			reset_game();
			osTimerStart(timer1, TIMER_PERIOD);
		}
	}
}
osThreadDef(thread_painel, osPriorityNormal, 1, 0);

void thread_pilulas(void const *argument){
	int i = 0, j = 0, k = 0, m = 0, original_score;
	while(1) {
		osSignalWait(0x01, osWaitForever);
		original_score = score;
		
		if(pac_vel_x==1)
		{
			for(i=0;i<7;i++)
			{
				if(coll_mtx[pacx+6][pacy+i]==2)
				{
					while(coll_mtx[pacx+6+j][pacy+i]==2)
					{
						coll_mtx[pacx+6+j][pacy+i]=1;
						j++;
					}
					j=0;
					score++;
				}
				else if(coll_mtx[pacx+6][pacy+i]==3)
				{
					coll_mtx[pacx+6][pacy+i]=1;
					touchedVitamin = true;
				}
				else if(coll_mtx[pacx+6][pacy+i]==5)
				{
					big_pill_enabled = false;
					for(i=60;i<8;i++)
					{
						for(j=50;j<5;j++)
							coll_mtx[i][j] = 1;
					}
					erase_big_pill();
					score+=150;
				}
			}
		}
		else if(pac_vel_x==-1)
		{
			for(i=0;i<7;i++)
			{
				if(coll_mtx[pacx][pacy+i]==2)
				{
					while(coll_mtx[pacx-j][pacy+i]==2)
					{
						coll_mtx[pacx-j][pacy+i]=1;
						j++;
					}
					j=0;
					score++;
				}
				else if(coll_mtx[pacx][pacy+i]==3)
				{
					coll_mtx[pacx][pacy+i]=1;
					touchedVitamin = true;
				}
				else if(coll_mtx[pacx][pacy+i]==5)
				{
					big_pill_enabled = false;
					for(i=60;i<8;i++)
					{
						for(j=50;j<5;j++)
							coll_mtx[i][j] = 1;
					}
					erase_big_pill();
					score+=150;
				}
			}
		}
		else if(pac_vel_y==1)
		{
			for(i=0;i<7;i++)
			{
				if(coll_mtx[pacx+i][pacy+6]==2)
				{
					while(coll_mtx[pacx+i+j][pacy+6]==2)
					{
						coll_mtx[pacx+i+j][pacy+6]=1;
						j++;
					}
					j=1;
					while(coll_mtx[pacx+i-j][pacy+6]==2)
					{
						coll_mtx[pacx+i-j][pacy+6]=1;
						j++;
					}
					j=0;
					score++;
				}
				else if(coll_mtx[pacx+i][pacy+6]==3)
				{
					coll_mtx[pacx+i][pacy+6]=1;
					touchedVitamin = true;
				}
				else if(coll_mtx[pacx+i][pacy+6]==5)
				{
					big_pill_enabled = false;
					for(i=60;i<8;i++)
					{
						for(j=50;j<5;j++)
							coll_mtx[i][j] = 1;
					}
					erase_big_pill();
					score+=150;
				}
			}
		}
		else if(pac_vel_y==-1)
		{
			for(i=0;i<7;i++)
			{
				if(coll_mtx[pacx+i][pacy]==2)
				{
					while(coll_mtx[pacx+i+j][pacy]==2)
					{
						coll_mtx[pacx+i+j][pacy]=1;
						j++;
					}
					j=1;
					while(coll_mtx[pacx+i-j][pacy]==2)
					{
						coll_mtx[pacx+i-j][pacy]=1;
						j++;
					}
					j=0;
					score++;
				}
				else if(coll_mtx[pacx+i][pacy]==3)
				{
					coll_mtx[pacx+i][pacy]=1;
					touchedVitamin = true;
				}
				else if(coll_mtx[pacx+i][pacy]==5)
				{
					big_pill_enabled = false;
					for(i=60;i<8;i++)
					{
						for(j=50;j<5;j++)
							coll_mtx[i][j] = 1;
					}
					erase_big_pill();
					score+=150;
				}
			}
		}
		
		if(big_pill_enabled == false)
		{
			if(rand()%5000000 < 5)
			{
				for(i=60;i<8;i++)
				{
					for(j=50;j<5;j++)
						coll_mtx[i][j] = 5;
				}
				draw_big_pill();
				startTick_big_pill = osKernelSysTick();
			}
		}
		else if(big_pill_enabled == true && (((osKernelSysTick() - startTick_big_pill) / (float)osKernelSysTickFrequency) * 145.0) > 3000)
		{
			big_pill_enabled = false;
			erase_big_pill();
			for(i=60;i<8;i++)
			{
				for(j=50;j<5;j++)
					coll_mtx[i][j] = 1;
			}
		}
		
		//se alterar a pontuação
		if(original_score != score && !touchedVitamin) {
			buzzer_per = 1;
			buzzer_per_set(buzzer_per);
			buzzer_write(true);
			print_score();
		}
		else
			buzzer_write(false);
		
		osSignalSet(thread_painel_id,0x02);
		
	}
}
osThreadDef(thread_pilulas, osPriorityNormal, 1, 0);

/*----------------------------------------------------------------------------
					Temporizadores
-------------------------------------------------------------------------------*/
void callback(void) {
	//acorda a interacao com usuario
	osSignalSet(thread_int_usuario_id,0x01);
	osSignalSet(thread_fantasmas_id, 0x01);
}
osTimerDef(periodo, callback);

/*----------------------------------------------------------------------------
 *      Main
 *---------------------------------------------------------------------------*/
int main (void) {
	uint32_t startTick = 0;
	
	// Initialize the kernel
	osKernelInitialize();
	startTick = osKernelSysTick();
	
	//Initializing all peripherals
	init_all();
	init_display_context();
	
	//Create the mutex
	mutex_display = osMutexCreate(osMutex(mutex_display));
	
	// Create the threads
	thread_int_usuario_id = osThreadCreate(osThread(thread_int_usuario), NULL);
	thread_fantasmas_id   = osThreadCreate(osThread(thread_fantasmas), NULL);
	thread_pacman_id      = osThreadCreate(osThread(thread_pacman), NULL);
	thread_painel_id      = osThreadCreate(osThread(thread_painel), NULL);
	thread_pilulas_id     = osThreadCreate(osThread(thread_pilulas), NULL);
	
	//Timer	
	timer1 = osTimerCreate(osTimer(periodo), osTimerPeriodic, NULL);	

	// Initalize collision matrix
	init_coll_mtx();
	
	// Draw static elements (map, pills, scoreboard)
	print_initial_map();
	print_lives();
		
	while(!button_read_debounce_s1()){}
	osTimerStart(timer1, TIMER_PERIOD);
	
	srand(osKernelSysTick()-startTick);
	// Start the kernel
	osKernelStart();	
	osDelay(osWaitForever);
}