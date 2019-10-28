#include "NewPWM.h"

/****************************************
 * A Tiva TM4C1294 não pode usar o método padrão SysCtlClockGet().
 * A macro é redefinida para obter o clock do sistema
 * **************************************/
#ifndef __SysCtlClockGet
#define __SysCtlClockGet()	\
SysCtlClockFreqSet( 				\
	SYSCTL_XTAL_25MHZ	| 			\
	SYSCTL_OSC_MAIN 	| 			\
	SYSCTL_USE_PLL 		| 			\
	SYSCTL_CFG_VCO_480, 			\
	120000000)
#endif

/********************************************************
 * Macros para configuração dos geradores e suas saídas,
 * utilizadas pela API.
 * *******************************************************/

#define PWM_GEN_BADDR(_mod_, _gen_)                                           \
                                ((_mod_) + (_gen_))
#define PWM_GEN_EXT_BADDR(_mod_, _gen_)                                       \
                                ((_mod_) + PWM_GEN_EXT_0 +                    \
                                 ((_gen_) - PWM_GEN_0) * 2)
#define PWM_OUT_BADDR(_mod_, _out_)                                           \
                                ((_mod_) + ((_out_) & 0xFFFFFFC0))
#define PWM_IS_OUTPUT_ODD(_out_)                                              \
                                ((_out_) & 0x00000001)



/* ----------- Variáveis globais ----------- */

uint32_t sysClock;
uint32_t current_period;
uint16_t current_amplitude;

/* ----------- Funções de manipulação dos módulos PWM ----------- */

void pwmInit() {
	sysClock = __SysCtlClockGet();

	// Ativa o periférico do PWM
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));

	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);

	// Ajusta a frequência do clock do PWM
	pwmClockConfig(FREQ_6);

  GPIOPinConfigure(GPIO_PF2_M0PWM2);

	// Configura o PWM 1 em modo countdown, com as duas saídas dessincronizadas
	// Somente uma saída é utilizada
	PWMGenConfigure_New(PWM0_BASE, PWM_GEN_1, (PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC));

	// Desativa inversão de sinal do PWM
	PWMOutputInvert_New(PWM0_BASE, PWM_OUT_2_BIT, false);

  pwmPeriodSet(10);
  pwmAmplitudeSet(0.35);

	// Ativa o PWM
	PWMGenEnable_New(PWM0_BASE, PWM_GEN_1);
  PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, true);
}

void pwmAmplitudeSet(float amplitude) {
  uint16_t pulse_width;
	uint32_t period;
	current_amplitude = amplitude;
	
	// Obtém o período do gerador PWM
	period = PWMGenPeriodGet_New(PWM0_BASE, PWM_GEN_1);
	
	pulse_width = amplitude * period;
	if(pulse_width >= period) pulse_width--; 
	if(pulse_width == 0) pulse_width++;
	
	// Configura o PWM para a largura de pulso informada
	PWMPulseWidthSet_New(PWM0_BASE, PWM_OUT_2, pulse_width);
}

void pwmPeriodSet(uint16_t period) {
	if(period < 0)
		period = 1;
	
	current_period = (uint16_t)(((float)period)*1000 / 8.32);

  // Desativa o PWM
	PWMGenDisable_New(PWM0_BASE, PWM_GEN_1);

	PWMGenPeriodSet_New(PWM0_BASE, PWM_GEN_1, current_period);
	pwmAmplitudeSet(current_amplitude);
	
  // Ativa o PWM
  PWMGenEnable_New(PWM0_BASE, PWM_GEN_1);
}

void pwmSet(uint16_t period, float amplitude) {
	pwmPeriodSet(period);
	pwmAmplitudeSet(amplitude);
}

static void pwmClockConfig(freq_t div_freq) {	
	switch(div_freq){
		case FREQ_0:
			PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_64);			
		break;
		case FREQ_1:
			PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_32);
		break;
		case FREQ_2:
			PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_16);
		break;
		case FREQ_3:
			PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_8);
		break;
		case FREQ_4:
				PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_4);
		break;
		case FREQ_5:
				PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_2);
		break;
		case FREQ_6:
				PWMClockSet(PWM0_BASE, PWM_SYSCLK_DIV_1);
	}
}
/* ----------- Funções de configuração dos módulos PWM ----------- */

// Configura o gerador PWM
// uint32_t ui32Base: endereço base do módulo PWM
// uint32_t ui32Gen: offset da saída do gerador PWM
// uinr32_t ui32Config: bits de configuração do gerador PWM
void PWMGenConfigure_New(uint32_t ui32Base, uint32_t ui32Gen, uint32_t ui32Config) {
    // Endereço base do gerador
    ui32Gen = PWM_GEN_BADDR(ui32Base, ui32Gen);

    // Altera configuração global do gerador
    HWREG(ui32Gen + PWM_O_X_CTL) = ((HWREG(ui32Gen + PWM_O_X_CTL) &
                                     ~(PWM_X_CTL_MODE | PWM_X_CTL_DEBUG |
                                       PWM_X_CTL_LATCH | PWM_X_CTL_MINFLTPER |
                                       PWM_X_CTL_FLTSRC |
                                       PWM_X_CTL_DBFALLUPD_M |
                                       PWM_X_CTL_DBRISEUPD_M |
                                       PWM_X_CTL_DBCTLUPD_M |
                                       PWM_X_CTL_GENBUPD_M |
                                       PWM_X_CTL_GENAUPD_M |
                                       PWM_X_CTL_LOADUPD | PWM_X_CTL_CMPAUPD |
                                       PWM_X_CTL_CMPBUPD)) | ui32Config);

    // Configura os geradores individuais em modo up/down ou countdown
	  // Modo up/down
    if(ui32Config & PWM_X_CTL_MODE) {
        HWREG(ui32Gen + PWM_O_X_GENA) = (PWM_X_GENA_ACTCMPAU_ONE |
                                         PWM_X_GENA_ACTCMPAD_ZERO);
        HWREG(ui32Gen + PWM_O_X_GENB) = (PWM_X_GENB_ACTCMPBU_ONE |
                                         PWM_X_GENB_ACTCMPBD_ZERO);
    }

	// Modo countdown
    else {
        HWREG(ui32Gen + PWM_O_X_GENA) = (PWM_X_GENA_ACTLOAD_ONE |
                                         PWM_X_GENA_ACTCMPAD_ZERO);
        HWREG(ui32Gen + PWM_O_X_GENB) = (PWM_X_GENB_ACTLOAD_ONE |
                                         PWM_X_GENB_ACTCMPBD_ZERO);
    }
}

// Configura inversão do sinal PWM
// uint32_t ui32Base: endereço base do gerador PWM
// uint32_t ui32PWMOutBits: saídas do PWM a serem alteradas
// bool bInvert: comando para inverter, ou não, as saídas informadas
void PWMOutputInvert_New(uint32_t ui32Base, uint32_t ui32PWMOutBits, bool bInvert) {
    // Limpa ou ativa os bits de inversão do sinal de acordo com o parâmetro bInvert
    if(bInvert == true) {
        HWREG(ui32Base + PWM_O_INVERT) |= ui32PWMOutBits;
    }
    else {
        HWREG(ui32Base + PWM_O_INVERT) &= ~(ui32PWMOutBits);
    }
}

// Ativa o gerador PWM informado
// uint32_t ui32Base: endereço base do módulo PWM
// uint32_t ui32Gen: saída do gerador PWM a ser ativada
void PWMGenEnable_New(uint32_t ui32Base, uint32_t ui32Gen) {
    // Escreve nos registradores que ativam o PWM
    HWREG(PWM_GEN_BADDR(ui32Base, ui32Gen) + PWM_O_X_CTL) |= PWM_X_CTL_ENABLE;
}

// Desativa o gerador PWM informado
void PWMGenDisable_New(uint32_t ui32Base, uint32_t ui32Gen) {
    // Desativa o gerador PWM
    HWREG(PWM_GEN_BADDR(ui32Base, ui32Gen) + PWM_O_X_CTL) &=
        ~(PWM_X_CTL_ENABLE);
}

// Retorna o período do gerador PWM informado
// uint32_t ui32Base: endereço base do módulo PWM
// uint32_t ui32Gen: offset do gerador PWM
uint32_t PWMGenPeriodGet_New(uint32_t ui32Base, uint32_t ui32Gen) {
    // Calcula o endereço base do gerador PWM
    ui32Gen = PWM_GEN_BADDR(ui32Base, ui32Gen);

    // Interpreta o modo de contagem (up/down ou countdown)
    if(HWREG(ui32Gen + PWM_O_X_CTL) & PWM_X_CTL_MODE) {
        // O período é o dobro do load register
        return(HWREG(ui32Gen + PWM_O_X_LOAD) * 2);
    }
    else {
        // O período é o load register + 1
        return(HWREG(ui32Gen + PWM_O_X_LOAD) + 1);
    }
}

// Configura o período do gerador PWM informado
void PWMGenPeriodSet_New(uint32_t ui32Base, uint32_t ui32Gen, uint32_t ui32Period) {
    // Calcula o endereço base do gerador
    ui32Gen = PWM_GEN_BADDR(ui32Base, ui32Gen);

    // Configura o load register baseado no modo de contagem
    if(HWREG(ui32Gen + PWM_O_X_CTL) & PWM_X_CTL_MODE) {
        // Metade do período informado para o modo up/down
        //ASSERT((ui32Period / 2) < 65536);
        HWREG(ui32Gen + PWM_O_X_LOAD) = ui32Period / 2;
    }
    else {
        // Período informado menos um, para o modo countdown
        //ASSERT((ui32Period <= 65536) && (ui32Period != 0));
        HWREG(ui32Gen + PWM_O_X_LOAD) = ui32Period - 1;
    }
}

// Configura a largura do pulso do gerador PWM informadp=o
void PWMPulseWidthSet_New(uint32_t ui32Base, uint32_t ui32PWMOut, uint32_t ui32Width) {
    uint32_t ui32GenBase, ui32Reg;

    // Calcula o endereço base do gerador
    ui32GenBase = PWM_OUT_BADDR(ui32Base, ui32PWMOut);

    // Se o contador está em modo up/down, divide o pulso por dois
    if(HWREG(ui32GenBase + PWM_O_X_CTL) & PWM_X_CTL_MODE) {
        ui32Width /= 2;
    }

	// Obtém o período
    ui32Reg = HWREG(ui32GenBase + PWM_O_X_LOAD);

    // Garante que o período não é muito grande
    //ASSERT(ui32Width < ui32Reg);

    // Calcula o valor de comparação
    ui32Reg = ui32Reg - ui32Width;

    // Escreve nos registradores apropriados
    if(PWM_IS_OUTPUT_ODD(ui32PWMOut)) {
        HWREG(ui32GenBase + PWM_O_X_CMPB) = ui32Reg;
    }
    else {
        HWREG(ui32GenBase + PWM_O_X_CMPA) = ui32Reg;
    }
}

// Configura o clock do módulo PWM
void PWMClockSet_New(uint32_t ui32Base, uint32_t ui32Config) {
    // Escreve o clock do PWM no registrador correspondente
    HWREG(ui32Base + PWM_O_CC) = ((HWREG(ui32Base + PWM_O_CC) &
                                   ~(PWM_CC_USEPWM | PWM_CC_PWMDIV_M)) |
                                  ui32Config);
}
