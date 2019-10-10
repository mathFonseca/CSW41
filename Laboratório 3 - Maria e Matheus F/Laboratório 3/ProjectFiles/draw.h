#include "cmsis_os.h"
#include "TM4C129.h"                    // Device header
#include <stdbool.h>
#include "grlib/grlib.h"
#include "cfaf128x128x16.h"

// Sprites
#include "../sprites_low_res/ppm/pacman0.h"
#include "../sprites_low_res/ppm/pacman1.h"
#include "../sprites_low_res/ppm/pacman2.h"
#include "../sprites_low_res/ppm/ghost0.h"
#include "../sprites_low_res/ppm/ghost1.h"
#include "../sprites_low_res/ppm/mapaParaColisoes.h"
#include "../sprites_low_res/ppm/vitamina.h"
#include "../sprites_low_res/ppm/eyes.h"

#include "../sprites_low_res/ppm/deadPacman/deadPacman0.h"
#include "../sprites_low_res/ppm/deadPacman/deadPacman1.h"
#include "../sprites_low_res/ppm/deadPacman/deadPacman2.h"
#include "../sprites_low_res/ppm/deadPacman/deadPacman3.h"
#include "../sprites_low_res/ppm/deadPacman/deadPacman4.h"
#include "../sprites_low_res/ppm/deadPacman/deadPacman5.h"
#include "../sprites_low_res/ppm/deadPacman/deadPacman6.h"

// Constants
#define SPRITEW 7
#define SPRITEH 7
#define PAC_INITIAL_X 60
#define PAC_INITIAL_Y 62
#define GHOST_INITIAL_X 60
#define GHOST_INITIAL_Y 37
#define GHOST_RET_POINT_X 60
#define GHOST_RET_POINT_Y 37
#define TIMER_PERIOD 1000/2

//To print on the screen
extern tContext sContext;

// Pacman
extern int pacx, pacy;
extern int pac_dir;
extern int pac_vel_x, pac_vel_y;
extern int pacman_sprite;
extern bool pacman_dead;
extern bool teleporting;
extern const unsigned char* pacman_sprites[];
extern const unsigned char* pacman_death_sprites[];

// Ghosts
extern uint8_t ghost_pos_x[];
extern uint8_t ghost_pos_y[];
extern uint8_t ghost_vel_x[];
extern uint8_t ghost_vel_y[];
extern int ghost_sprite;
extern const unsigned char* ghost_sprites[];
extern int ghost_state[];

// Pills
extern bool big_pill_enabled;
extern uint32_t startTick_big_pill;

//Mutex
extern osMutexId mutex_display;

// Collision matrix
extern uint8_t coll_mtx[128][128];

// Score and lives
extern int score;
extern uint8_t lives;;

// Utility functions
static void intToString(int64_t value, char * pBuf, uint32_t len, uint32_t base, uint8_t zeros);

/****************************
 **** Rotinas de desenho ****
 ****************************/
void print_scores();
void print_lives();
void clear_display_sprite(int velx, int vely, int posx, int posy);
void print_fantasmas();
void print_pacman();
void print_initial_map();
void draw_pixel(int x, int y);
void draw_big_pill();
void erase_big_pill();
