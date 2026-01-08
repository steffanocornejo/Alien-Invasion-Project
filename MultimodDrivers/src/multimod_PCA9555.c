// multimod_OPT3001.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for OPT3001 functions

/************************************Includes***************************************/

#include "../multimod_PCA9555.h"

#include <stdint.h>
#include "../multimod_i2c.h"

/************************************Includes***************************************/

/********************************Public Functions***********************************/

uint16_t PCA9555_GetInput(uint32_t mod, uint8_t addr) {
    return PCA9555_ReadReg(mod, addr, PCA9555_INPUT_ADDR);
}

void PCA9555_SetPinDir(uint32_t mod, uint8_t addr, uint16_t pins) {
    uint8_t data[3];

    // Prepare data: [Command Byte, Port0, Port1]
    data[0] = PCA9555_CONFIG_ADDR;
    data[1] = (uint8_t)(pins & 0xFF);        // Port 0 (lower byte)
    data[2] = (uint8_t)((pins >> 8) & 0xFF); // Port 1 (upper byte)

    // Write all 3 bytes at once
    I2C_WriteMultiple(mod, addr, data, 3);
}

void PCA9555_SetPinPol(uint32_t mod, uint8_t addr, uint16_t pins) {
    uint8_t data[3];

    data[0] = PCA9555_POLINV_ADDR;
    data[1] = (uint8_t)(pins & 0xFF);        // Port 0 (lower byte)
    data[2] = (uint8_t)((pins >> 8) & 0xFF); // Port 1 (upper byte)

    // Write all 3 bytes at once
    I2C_WriteMultiple(mod, addr, data, 3);
}
void PCA9555_SetOutput(uint32_t mod, uint8_t addr, uint16_t pins) {
    uint8_t data[3];

    data[0] = PCA9555_OUTPUT_ADDR;
    data[1] = (uint8_t)(pins & 0xFF);        // Port 0 (lower byte)
    data[2] = (uint8_t)((pins >> 8) & 0xFF); // Port 1 (upper byte)

    // Write all 3 bytes at once
    I2C_WriteMultiple(mod, addr, data, 3);
}



uint16_t PCA9555_ReadReg(uint32_t mod, uint8_t gpio_module_addr, uint8_t addr) {
    uint8_t data[2];

    I2C_WriteSingle(mod, gpio_module_addr, addr);

    SysCtlDelay(100);

   // Read 2 bytes (Port 0 and Port 1)
    I2C_ReadMultiple(mod, gpio_module_addr, data, 2);

    // Pack as 16-bit: Port1 (MSB) | Port0 (LSB)
    return (uint16_t)((data[1] << 8) | data[0]);
}

/********************************Public Functions***********************************/
