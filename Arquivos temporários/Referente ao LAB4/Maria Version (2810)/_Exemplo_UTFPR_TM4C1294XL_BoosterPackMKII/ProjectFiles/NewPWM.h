#include <stdint.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_memmap.h"
#include "inc/hw_pwm.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
// #include "driverlib/timer.h"
#include "driverlib/rom_map.h"

// Define os endereços dos geradores PWM
#define PWM_GEN_0               0x00000040  // Offset do endereço do gerador 0
#define PWM_GEN_1               0x00000080  // Offset do endereço do gerador 1
#define PWM_GEN_2               0x000000C0  // Offset do endereço do gerador 2
#define PWM_GEN_3               0x00000100  // Offset do endereço do gerador 3
#define PWM_GEN_0_BIT           0x00000001  // Bit-wise ID for Gen0
#define PWM_GEN_1_BIT           0x00000002  // Bit-wise ID for Gen1
#define PWM_GEN_2_BIT           0x00000004  // Bit-wise ID for Gen2
#define PWM_GEN_3_BIT           0x00000008  // Bit-wise ID for Gen3
#define PWM_GEN_EXT_0           0x00000800  // Offset of Gen0 ext address range
#define PWM_GEN_EXT_1           0x00000880  // Offset of Gen1 ext address range
#define PWM_GEN_EXT_2           0x00000900  // Offset of Gen2 ext address range
#define PWM_GEN_EXT_3           0x00000980  // Offset of Gen3 ext address range

// Define os endereços das saídas dos PWM
#define PWM_OUT_0               0x00000040  // Offset do endereço do PWM0
#define PWM_OUT_1               0x00000041  // Offset do endereço do PWM1
#define PWM_OUT_2               0x00000082  // Offset do endereço do PWM2
#define PWM_OUT_3               0x00000083  // Offset do endereço do PWM3
#define PWM_OUT_4               0x000000C4  // Offset do endereço do PWM4
#define PWM_OUT_5               0x000000C5  // Offset do endereço do PWM5
#define PWM_OUT_6               0x00000106  // Offset do endereço do PWM6
#define PWM_OUT_7               0x00000107  // Offset do endereço do PWM7
#define PWM_OUT_0_BIT           0x00000001  // Bit-wise ID do PWM0
#define PWM_OUT_1_BIT           0x00000002  // Bit-wise ID do PWM1
#define PWM_OUT_2_BIT           0x00000004  // Bit-wise ID do PWM2
#define PWM_OUT_3_BIT           0x00000008  // Bit-wise ID do PWM3
#define PWM_OUT_4_BIT           0x00000010  // Bit-wise ID do PWM4
#define PWM_OUT_5_BIT           0x00000020  // Bit-wise ID do PWM5
#define PWM_OUT_6_BIT           0x00000040  // Bit-wise ID do PWM6
#define PWM_OUT_7_BIT           0x00000080  // Bit-wise ID do PWM7

// Defines dos bits de configuração do PWM
#define PWM_GEN_MODE_DOWN       0x00000000  // Modo do timer countdown
#define PWM_GEN_MODE_NO_SYNC    0x00000000  // Atualizações imediatas e assincronia entre geradores

// Defines das divisões de clock dos PWMs
#define PWM_SYSCLK_DIV_1        0x00000000  // PWM clock is system clock
#define PWM_SYSCLK_DIV_2        0x00000100  // PWM clock is system clock /2
#define PWM_SYSCLK_DIV_4        0x00000101  // PWM clock is system clock /4
#define PWM_SYSCLK_DIV_8        0x00000102  // PWM clock is system clock /8
#define PWM_SYSCLK_DIV_16       0x00000103  // PWM clock is system clock /16
#define PWM_SYSCLK_DIV_32       0x00000104  // PWM clock is system clock /32
#define PWM_SYSCLK_DIV_64       0x00000105  // PWM clock is system clock /64

// Macro para acesso de hardware
#define HWREG(x) (*((volatile uint32_t *)(x)))

typedef enum {FREQ_0, FREQ_1, FREQ_2, FREQ_3, FREQ_4, FREQ_5, FREQ_6} 
freq_t;

/* ------------- Funções de utilização do PWM (API) ------------- */
void pwmInit();
void pwmAmplitudeSet(float amplitude);
void pwmPeriodSet(uint16_t period);
void pwmSet(uint16_t period, float amplitude);
static void pwmClockConfig(freq_t div_freq);

/* ------------- Funções de configuração do PWM ------------- */
void PWMGenConfigure_New(uint32_t ui32Base, uint32_t ui32Gen, uint32_t ui32Config);
void PWMGenEnable_New(uint32_t ui32Base, uint32_t ui32Gen);
void PWMGenDisable_New(uint32_t ui32Base, uint32_t ui32Gen);
void PWMOutputInvert_New(uint32_t ui32Base, uint32_t ui32PWMOutBits, bool bInvert);
uint32_t PWMGenPeriodGet_New(uint32_t ui32Base, uint32_t ui32Gen);
void PWMGenPeriodSet_New(uint32_t ui32Base, uint32_t ui32Gen, uint32_t ui32Period);
void PWMPulseWidthSet_New(uint32_t ui32Base, uint32_t ui32PWMOut, uint32_t ui32Width);
void PWMClockSet_New(uint32_t ui32Base, uint32_t ui32Config);
