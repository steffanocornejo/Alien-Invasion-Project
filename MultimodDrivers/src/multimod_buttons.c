// multimod_buttons.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for button functions

/************************************Includes***************************************/

#include "../multimod_buttons.h"

#include "../multimod_i2c.h"

#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <driverlib/i2c.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_i2c.h>

// The 7-bit I2C address for the PCA9555PW on the LED module
#define BUTTON_I2C_ADDR   0x23

// PCA9555 Register for "Input Port 0" (where buttons are)
#define REG_INPUT_PORT_0    0x00

// PCA9555 Register for "Configuration Port 0"
#define REG_CONFIG_PORT_0   0x06

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// Buttons_Init
// Initializes buttons on the multimod by configuring the I2C module and
// relevant interrupt pin.
// Return: void
void MultimodButtons_Init() {
   I2C_Init(I2C1_BASE);

   SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
   GPIOPinTypeGPIOInput(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);

   GPIOPadConfigSet(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);
   GPIOIntClear(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
   GPIOIntTypeSet(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN, GPIO_FALLING_EDGE);
   GPIOIntEnable(BUTTONS_INT_GPIO_BASE, BUTTONS_INT_PIN);
   uint8_t bytes[3] = {0x06, (SW1 | SW2 | SW3 | SW4), 0xFF};
   I2C_WriteMultiple(I2C_A_BASE, BUTTONS_PCA9555_GPIO_ADDR, bytes, 3);

}

// MultimodButtons_Get
// Gets the input to GPIO bank 1, [0..7].
// Return: uint8_t 
uint8_t MultimodButtons_Get() {
    I2C_WriteSingle(I2C_A_BASE, BUTTONS_PCA9555_GPIO_ADDR, 0);
    uint8_t data = I2C_ReadSingle(I2C_A_BASE, BUTTONS_PCA9555_GPIO_ADDR);
   return (uint8_t)(~data) & (SW1 | SW2 | SW3 | SW4);
}

/********************************Public Functions***********************************/

