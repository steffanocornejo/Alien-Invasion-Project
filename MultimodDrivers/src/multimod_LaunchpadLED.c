// multimod_OPT3001.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for OPT3001 functions

/************************************Includes***************************************/

#include "../multimod_LaunchpadLED.h"

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

#include <driverlib/pin_map.h>
#include <driverlib/pwm.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// LaunchpadButtons_Init
// Initializes the GPIO port & pins necessary for the button switches on the
// launchpad. Also configures it so that the LEDs are controlled via PWM signal.
// Initial default period of 400.
// Return: void
void LaunchpadLED_Init() {

       // 1. Enable the clock to the GPIO Port F and PWM Module 1
       SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
       SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1);
       while(!SysCtlPeripheralReady(SYSCTL_PERIPH_PWM1)); // Wait for PWM1 to be ready

       HWREG(GPIO_PORTF_BASE + GPIO_O_LOCK) = GPIO_LOCK_KEY;
       HWREG(GPIO_PORTF_BASE + GPIO_O_CR) |= (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

       // 2. Configure the GPIO pins for their PWM function.
       // PF1 is Red (M1PWM5), PF2 is Blue (M1PWM6), PF3 is Green (M1PWM7)
       GPIOPinConfigure(GPIO_PF1_M1PWM5);
       GPIOPinConfigure(GPIO_PF2_M1PWM6);
       GPIOPinConfigure(GPIO_PF3_M1PWM7);
       GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

       // 3. Configure the PWM generators for the LEDs.

       PWMGenConfigure(PWM1_BASE, PWM_GEN_2, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);
       PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN | PWM_GEN_MODE_NO_SYNC);

       PWMGenPeriodSet(PWM1_BASE, PWM_GEN_2, 400);
       PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, 400);

       // 5. Set the initial pulse width (duty cycle) for each LED.
       // Let's start with the LEDs off (pulse width near zero).
       PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, 1); // Red LED (PF1)
       PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, 1); // Blue LED (PF2)
       PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, 1); // Green LED (PF3)

       // 6. Enable the PWM outputs.
       PWMOutputState(PWM1_BASE, PWM_OUT_5_BIT | PWM_OUT_6_BIT | PWM_OUT_7_BIT, true);

       // 7. Enable the PWM generators. This should be done *after* configuration.
       PWMGenEnable(PWM1_BASE, PWM_GEN_2);
       PWMGenEnable(PWM1_BASE, PWM_GEN_3);

}

// LaunchpadLED_PWMSetDuty
// Sets the duty cycle of the PWM generator associated with the LED.
// Return: void
uint32_t pulse_width;

void LaunchpadLED_PWMSetDuty(LED_Color_t LED, float duty) {
    pulse_width = (duty * PWM_Per);

    // If pulse width < 1, set as 1
    if (pulse_width < 1) {
            pulse_width = 1;
        }
        if (pulse_width >= PWM_Per) {
            pulse_width = PWM_Per - 1; // Cap at max value
        }

        // Set the PWM for the chosen LED
        if (LED == RED) {
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_5, pulse_width); // Red LED (PF1)
        }
        else if (LED == BLUE) {
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, pulse_width); // Blue LED (PF2)
        }
        else if (LED == GREEN) {
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, pulse_width); // Green LED (PF3)
        }


    return;
}

/********************************Public Functions***********************************/
