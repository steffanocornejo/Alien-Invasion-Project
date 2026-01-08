// multimod_uart.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for UART functions

/************************************Includes***************************************/


#define BB_UART_BASE            UART4_BASE
#define BB_UART_PERIPH_UART     SYSCTL_PERIPH_UART4
#define BB_UART_PERIPH_GPIO     SYSCTL_PERIPH_GPIOC
#define BB_UART_GPIO_BASE       GPIO_PORTC_BASE
#define BB_UART_RX_PINCONF      GPIO_PC4_U4RX
#define BB_UART_TX_PINCONF      GPIO_PC5_U4TX
#define BB_UART_RX_PIN          GPIO_PIN_4
#define BB_UART_TX_PIN          GPIO_PIN_5
#define BB_UART_INT


#include "../multimod_uart.h"

#include <stdint.h>
#include <stdbool.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

//#include <uartstdio.h>
#include <driverlib/gpio.h>
#include <driverlib/uart.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/


// UART_Init
// Initializes UART serial communication with PC
// Return: void
void UART_Init() {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));

    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0));

    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200,
                        (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));

    UARTStdioConfig(0, 115200, SysCtlClockGet());
}



void UART_BeagleBone_init() {

    SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    SysCtlPeripheralEnable(BB_UART_PERIPH_GPIO);
    while(!SysCtlPeripheralReady(BB_UART_PERIPH_GPIO)) {}
    SysCtlPeripheralEnable(BB_UART_PERIPH_UART);
    while(!SysCtlPeripheralReady(BB_UART_PERIPH_UART)) {}

    UARTEnable(BB_UART_BASE);
    UARTFIFOEnable(BB_UART_BASE);

    GPIOPinConfigure(BB_UART_RX_PINCONF);   // pc4 U4RX
    GPIOPinConfigure(BB_UART_TX_PINCONF);   // pc5 U4TX
    GPIOPinTypeUART(BB_UART_GPIO_BASE, BB_UART_RX_PIN | BB_UART_TX_PIN);

    UARTClockSourceSet(UART4_BASE, UART_CLOCK_PIOSC);

    UARTConfigSetExpClk(BB_UART_BASE, SysCtlClockGet(), 115200, UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE);


}



/********************************Public Functions***********************************/

