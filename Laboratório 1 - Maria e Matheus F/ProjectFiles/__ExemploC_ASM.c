#include "TM4C129.h"                    // Device header
#include <stdio.h>
 
 
int matriz[4][4] = {{1,2,3,4},{5,6,7,8},{9,10,11,12},{13,14,15,16}};
 
extern int f_asm(void);
int main()
{
  int i;  
  uint8_t y;
	uint8_t *p = (uint8_t *) matriz;
 
  i = f_asm();
 
  for (y=0;y<100;y++){
        printf("%u\n",*p);
        p++;
  }    
 
  return 0;
}
 