// multimod_OPT3001.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for OPT3001 functions

/************************************Includes***************************************/

#include "../multimod_OPT3001.h"

#include <stdint.h>
#include "../multimod_i2c.h"

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// OPT3001_Init
// Initializes OPT3001, configures it to continuous conversion mode.
// Return: void
void OPT3001_Init(void) {
    I2C_Init(I2C_A_BASE);

    // normally would add a software reset here, however,
    // opt3001 does not support software resets.

    OPT3001_WriteRegister(OPT3001_CONFIG_ADDR, 0xC410);


    return;
}

// OPT3001_WriteRegister
// Writes to a register in the OPT3001.
// Param uint8_t "addr": Register address of the OPT3001.
// Param uint16_t "data": 16-bit data to write to the register.
// Return: void
void OPT3001_WriteRegister(uint8_t addr, uint16_t data) {
    // to where and with what?
    uint8_t buf[3];

    buf[0] = addr;            // register address
       buf[1] = (data >> 8) & 0xFF;  // MSB
       buf[2] = data & 0xFF;         // LSB

    I2C_WriteMultiple(I2C_A_BASE, OPT3001_ADDR, buf, 3);

    return;
}

// OPT3001_ReadRegister
// Reads from a register in the OPT3001.
// Param uint8_t "addr": Register address of the OPT3001.
// Return: uint16_t
uint16_t OPT3001_ReadRegister(uint8_t addr) {
    // from where into what?

    uint8_t data[2];

    I2C_WriteSingle(I2C_A_BASE, OPT3001_ADDR, addr); // writes the register address into OPT3001_ADDR

    I2C_ReadMultiple(I2C_A_BASE, OPT3001_ADDR, data, 2); // puts data  from OPT3001_ADDR into data
    
    return ((uint16_t)data[0] << 8) | data[1];
}

// OPT3001_GetInterrupt
// Gets interrupt flag bits from the config register of the OPT3001.
// Return: uint16_t
uint16_t OPT3001_GetInterrupt(void) {
    // free
    uint16_t flags = OPT3001_ReadRegister(OPT3001_CONFIG_ADDR);

    flags &= (OPT3001_FLAG_OVF | OPT3001_FLAG_CRF | OPT3001_FLAG_FH | OPT3001_FLAG_FL);

    return flags;
}

// OPT3001_GetInterrupt
// Gets conversion result, calculates byte result based on datasheet
// and configuration settings.
// Return: uint32_t
uint32_t OPT3001_GetResult(void) {
    // wait until ready

   // while(!(OPT3001_ReadRegister(OPT3001_CONFIG_ADDR) & (1 << 7)));

    uint32_t result;
    
    
    result = OPT3001_ReadRegister(OPT3001_RESULT_ADDR);

    // free
    result = LUX((result >> 12 & 0xF), (result & 0x0FFF));

    return result;
}

// OPT3001_SetConfig
// Sets the configuration register.
// Param uint16_t "config": configuration bits
// Return: void
void OPT3001_SetConfig(uint16_t config) {
    // where are "configs" normally sent to / stored for a device?



    return;
}

// OPT3001_SetLowLimit
// Sets the low limit register.
// Param uint16_t "exp": Exponential bound
// Param uint16_t "result": Conversion bound
// Return: void
void OPT3001_SetLowLimit(uint16_t exp, uint16_t result) {
    // free
    OPT3001_WriteRegister(OPT3001_LOWLIMIT_ADDR, (exp << OPT3001_RESULT_E_S | (result & 0xFFF)));

    return;
}

// OPT3001_SetHighLimit
// Sets the high limit register.
// Param uint16_t "exp": Exponential bound
// Param uint16_t "result": Conversion bound
// Return: void
void OPT3001_SetHighLimit(uint16_t exp, uint16_t result) {
    // free
    OPT3001_WriteRegister(OPT3001_HIGHLIMIT_ADDR, (exp << OPT3001_RESULT_E_S | (result & 0xFFF)));

    return;
}

// OPT3001_GetChipID
// Gets the chip ID of the OPT3001.
// Return: uint16_t
uint16_t OPT3001_GetChipID(void) {

    uint16_t data;

    // where is this data stored? 
    return data;
}

/********************************Public Functions***********************************/
