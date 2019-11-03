#include "PWM.h"

void PWM_function_init(void){
	uint32_t ui32Gen;
	/*----------------------------
	--------- GPIO ---------------
	----------------------------*/
	
	// Enable the PWM0 peripheral
	SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM0));
	
	// Wait for the PWM0 module to be ready
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
  
	// Configure PIN for use by the PWM peripheral
	GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 |GPIO_PIN_2);
	
	// Configures the alternate function of a GPIO pin
	// PF1_M0PWM1 --> piezo buzzer
	GPIOPinConfigure(GPIO_PF2_M0PWM2);
	
	/*----------------------------
	--------- PWM ---------------
	----------------------------*/
	//
	// Set the PWM clock configuration into the PWM clock configuration
	// register.
	//
	HWREG(PWM0_BASE + PWM_O_CC) = ((HWREG(PWM0_BASE + PWM_O_CC) &
																 ~(PWM_CC_USEPWM | PWM_CC_PWMDIV_M)) |
																PWM_SYSCLK_DIV_64);	

	
	// Configures a PWM generator.
	// This function is used to set the mode of operation for a PWM generator.  The counting mode,
	// synchronization mode, and debug behavior are all configured. After configuration, the generator is left in the disabled state.
	
	
	//PWMGenConfigure(PWM0_BASE, PWM_GEN_1, (PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC)); //PWM_GEN_1
	
	  //
    // Compute the generator's base address.
    //
		 ui32Gen = PWM_GEN_BADDR(PWM0_BASE, PWM_GEN_1);
	
	  //
    // Set the individual PWM generator controls.
    //
    if((PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC) & PWM_X_CTL_MODE)
    {
        //
        // In up/down count mode, set the signal high on up count comparison
        // and low on down count comparison (that is, center align the
        // signals).
        //
        HWREG(ui32Gen + PWM_O_X_GENA) = (PWM_X_GENA_ACTCMPAU_ONE |
                                         PWM_X_GENA_ACTCMPAD_ZERO);
        HWREG(ui32Gen + PWM_O_X_GENB) = (PWM_X_GENB_ACTCMPBU_ONE |
                                         PWM_X_GENB_ACTCMPBD_ZERO);
    }
    else
    {
        //
        // In down count mode, set the signal high on load and low on count
        // comparison (that is, left align the signals).
        //
        HWREG(ui32Gen + PWM_O_X_GENA) = (PWM_X_GENA_ACTLOAD_ONE |
                                         PWM_X_GENA_ACTCMPAD_ZERO);
        HWREG(ui32Gen + PWM_O_X_GENB) = (PWM_X_GENB_ACTLOAD_ONE |
                                         PWM_X_GENB_ACTCMPBD_ZERO);
    }
	
	
	
	
	// Set the PWM period to 500Hz.  To calculate the appropriate parameter
	// use the following equation: N = (1 / f) * SysClk.  Where N is the
	// function parameter, f is the desired frequency, and SysClk is the
	// system clock frequency.
	// In this case you get: (1 / 500) * 120MHz / 64 = 3750 cycles.  Note that
	// the maximum period you can set is 2^16.
	// TODO: modify this calculation to use the clock frequency that you are
	// using.
	
	///buzzer_per_set(0xFFFF);
		PWM_per_set(0xFFFF);
	//PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, 0xffff);
	///buzzer_vol_set(0x7FFF); 
		PWM_amplitude_set(0x7FFF);
	//PWMPulseWidthSet(PWM0_BASE, PWM_OUT_1, 0x7fff);
	///buzzer_write(false);
		PWM_enable(false);
	//PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, false);
	
	PWMOutputInvert(PWM0_BASE, PWM_OUT_2_BIT, false);
	PWMGenEnable(PWM0_BASE, PWM_GEN_1);
}
void PWM_enable(bool estado){
	g_current_state = estado;
	//PWMOutputState(PWM0_BASE, PWM_OUT_2_BIT, estado);
	
	if(estado == true)
	{
			HWREG(PWM0_BASE + PWM_O_ENABLE) |= PWM_OUT_2_BIT;
	}
	else
	{
			HWREG(PWM0_BASE + PWM_O_ENABLE) &= ~(PWM_OUT_2_BIT);
	}
	
	
	
}
void PWM_amplitude_set(uint16_t volume){
	uint16_t pulse_width;
	uint32_t period;
	uint32_t ui32Reg,ui32GenBase,ui32Gen;
	g_current_amp = volume;

	period = PWMGenPeriodGet(PWM0_BASE, PWM_GEN_1);

	pulse_width = volume*period/0xFFFF;
	if(pulse_width >= period) pulse_width--; 
	if(pulse_width <= period) pulse_width++;
	if(pulse_width == 0) pulse_width = 0;
	// Sets the pulse width for the specified PWM output
	PWMPulseWidthSet(PWM0_BASE, PWM_OUT_2, pulse_width);
	

	
	
}
void PWM_per_set(uint16_t period){
	bool last_state = g_current_state;
	uint32_t ui32Gen;
	
	if(period < 3)
		period = 3;
	g_current_per = period;
	PWM_enable(false);
	PWMGenPeriodSet(PWM0_BASE, PWM_GEN_1, period);
	PWM_amplitude_set(g_current_amp);
	PWM_enable(last_state);
}
