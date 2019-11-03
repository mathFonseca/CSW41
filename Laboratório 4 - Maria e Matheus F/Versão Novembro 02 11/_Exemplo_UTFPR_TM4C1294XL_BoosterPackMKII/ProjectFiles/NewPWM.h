#ifndef __PWM_H_
#define __PWM_H_
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_pwm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/pwm.h"
#include "inc/hw_types.h"
#include "driverlib/interrupt.h"
#include "inc/hw_sysctl.h"

//*****************************************************************************
//
// Misc macros for manipulating the encoded generator and output defines used
// by the API.
//
//*****************************************************************************
#define PWM_GEN_BADDR(_mod_, _gen_)                                           \
                                ((_mod_) + (_gen_))
#define PWM_GEN_EXT_BADDR(_mod_, _gen_)                                       \
                                ((_mod_) + PWM_GEN_EXT_0 +                    \
                                 ((_gen_) - PWM_GEN_0) * 2)
#define PWM_OUT_BADDR(_mod_, _out_)                                           \
                                ((_mod_) + ((_out_) & 0xFFFFFFC0))
#define PWM_IS_OUTPUT_ODD(_out_)                                              \
                                ((_out_) & 0x00000001)

#define ASSERT(expr)

#define PWM_FREQUENCY			500

#define PWM_SYSCLK_DIV_64       0x00000105  // PWM clock is system clock /64
#define PWM_GEN_0               0x00000040  // Offset address of Gen0
#define PWM_GEN_MODE_DOWN       0x00000000  // Down count mode
#define PWM_GEN_MODE_NO_SYNC    0x00000000  // Immediate updates



static uint32_t g_ui32SysClock = 120000000; // 120MHz
static uint32_t g_current_amp;
static uint16_t g_current_per;
static bool g_current_state;

void PWM_function_init(void);
void PWM_enable(bool estado);
void PWM_amplitude_set(uint16_t volume);
void PWM_per_set(uint16_t period);


#endif
