#ifndef MULTIMOD_H_
#define MULTIMOD_H_

#include "multimod_uart.h"
#include "multimod_i2c.h"
#include "multimod_BMI160.h"
#include "multimod_OPT3001.h"
#include "multimod_LaunchpadButtons.h"
#include "multimod_LaunchpadLED.h"
#include "multimod_joystick.h"
#include "multimod_buttons.h"
#include "multimod_spi.h"
#include "multimod_ST7789.h"


static void Multimod_Init() {
    BMI160_Init();
    OPT3001_Init();
    UART_Init();
    LaunchpadButtons_Init();
    MultimodButtons_Init();
    JOYSTICK_Init();
    ST7789_Init();
//    UART_BeagleBone_init();
}

#endif /* MULTIMOD_H_ */
