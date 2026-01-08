// multimod_joystick.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for joystick functions

/************************************Includes***************************************/

#include "../multimod_joystick.h"

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/adc.h>
#include <driverlib/interrupt.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_types.h>
#include <inc/hw_memmap.h>
#include <inc/hw_i2c.h>
#include <inc/hw_gpio.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// JOYSTICK_Init
// Initializes ports & adc module for joystick
// Return: void
void JOYSTICK_Init(void) {

        SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

        // 2. Configure PE2 (Y) and PE3 (X) as analog inputs
        GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2 | GPIO_PIN_3);

        // 3. Configure PD2 (Press) as a digital input with pull-up
        GPIOPinTypeGPIOInput(GPIO_PORTD_BASE, GPIO_PIN_2);
        GPIOPadConfigSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

        // 4. Configure ADC Sequencer 2 for 2 samples (X and Y)
        ADCSequenceConfigure(ADC0_BASE, 2, ADC_TRIGGER_PROCESSOR, 0);
        ADCSequenceStepConfigure(ADC0_BASE, 2, 0, ADC_CTL_CH1);
        ADCSequenceStepConfigure(ADC0_BASE, 2, 1, ADC_CTL_CH0 | ADC_CTL_IE | ADC_CTL_END);
        ADCSequenceEnable(ADC0_BASE, 2);

        // 5. Configure interrupt for the button press (PD2)
        GPIOIntTypeSet(GPIO_PORTD_BASE, GPIO_PIN_2, GPIO_FALLING_EDGE);
        GPIOIntClear(GPIO_PORTD_BASE, GPIO_PIN_2);
}

// JOYSTICK_IntEnable
// Enables interrupts
// Return: void
void JOYSTICK_IntEnable() {
    // Enable the pin-specific interrupt
    GPIOIntEnable(GPIO_PORTD_BASE, GPIO_PIN_2);

    // Enable the GPIO Port D interrupt in the NVIC
    IntEnable(INT_GPIOD);
}

// JOYSTICK_GetPress
// Gets button reading
// Return: bool
uint8_t JOYSTICK_GetPress() {
    return (GPIOPinRead(GPIO_PORTD_BASE, GPIO_PIN_2) == 0);
}

// JOYSTICK_GetX
// Gets X adc reading from joystick
// Return: uint16_t
uint16_t JOYSTICK_GetX() {
    // your code
    uint32_t xy = JOYSTICK_GetXY();
    return (uint16_t)(xy >> 16);
}

// JOYSTICK_GetY
// Gets Y adc reading from joystick
// Return: uint16_t
uint16_t JOYSTICK_GetY() {
    // your code
    uint32_t xy = JOYSTICK_GetXY();
    return (uint16_t)(xy & 0x0000FFFF);
}


// JOYSTICK_GetXY
// Gets X and Y adc readings
// Return: uint32_t, 16-bit packed, upper 16-bit is X and lower 16-bit is Y.
uint32_t JOYSTICK_GetXY() {
    // Array to hold the 2 values from the ADC FIFO
        uint32_t pui32ADC0Value[2];

        // Clear the interrupt flag
        ADCIntClear(ADC0_BASE, 2);

        // Trigger the ADC conversion
        ADCProcessorTrigger(ADC0_BASE, 2);

        // Wait for conversion to be completed
        while(!ADCIntStatus(ADC0_BASE, 2, false))
        {
        }

        // Clear the ADC interrupt flag
        ADCIntClear(ADC0_BASE, 2);

        // Read the values from the ADC FIFO
        ADCSequenceDataGet(ADC0_BASE, 2, pui32ADC0Value);

        // pui32ADC0Value[0] is AIN1 (Y)
        // pui32ADC0Value[1] is AIN0 (X)

        // Pack as (X << 16) | Y
        return (uint32_t)((pui32ADC0Value[1] << 16) | (pui32ADC0Value[0] & 0xFFFF));
}

/********************************Public Functions***********************************/

