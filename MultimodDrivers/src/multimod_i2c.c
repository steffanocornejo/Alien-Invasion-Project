// multimod_i2c.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for I2C functions

/************************************Includes***************************************/

#include "../multimod_i2c.h"
#include <stdbool.h>
#include <stdio.h>
#include <driverlib/gpio.h>
#include <driverlib/sysctl.h>
#include <driverlib/pin_map.h>
#include <inc/hw_memmap.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_i2c.h>

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// I2C_Init
// Initializes specified I2C module
// Param uint32_t "mod": base address of module
// Return: void
void I2C_Init(uint32_t mod) {
    // which I2C module?

    // does it require GPIO pins? if so, how do we configure them?
    if (I2C_A_BASE  == mod) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

        GPIOPinConfigure(GPIO_PA6_I2C1SCL); // Configuring both port b pins to I2C compatibility
        GPIOPinConfigure(GPIO_PA7_I2C1SDA);

        GPIOPinTypeI2CSCL(GPIO_PORTA_BASE, I2C_A_PIN_SCL); // configure pin on port a to SDA
        GPIOPinTypeI2C(GPIO_PORTA_BASE, I2C_A_PIN_SDA); // configure pin on port a to SCL

        I2CMasterInitExpClk(I2C_A_BASE, SysCtlClockGet(), false);
    }
    else {
        return;
    }

}

// I2C_WriteSingle
// Writes a single byte to an address.
// Param uint32_t "mod": base address of module
// Param uint8_t "addr": address to device
// Param uint8_t "byte": byte to send
// Return: void
void I2C_WriteSingle(uint32_t mod, uint8_t addr, uint8_t byte) {
    // which module and where to?
    I2CMasterSlaveAddrSet(mod, addr, false); // mod is writing to addr (False = write, True = read)


    // what data?
    I2CMasterDataPut(mod, byte); // writes the byte into I2C data register


    // what mode?

    I2CMasterControl(mod, I2C_MASTER_CMD_SINGLE_SEND); // sets the master to send a single byte of data
    // should we wait till finished?
    while(I2CMasterBusy(mod));


    return;
}

// I2C_ReadSingle
// Reads a single byte from address.
// Param uint32_t "mod": base address of module
// Param uint8_t "addr": address to device
// Return: uint8_t
uint8_t I2C_ReadSingle(uint32_t mod, uint8_t addr) {
    // which module and where to? 
    I2CMasterSlaveAddrSet(mod, addr, true); // mod is reading from addr (False = write, True = read)

    // what data?

    I2CMasterControl(mod, I2C_MASTER_CMD_SINGLE_RECEIVE);

    // what mode?

    while(I2CMasterBusy(mod));

    uint32_t data = I2CMasterDataGet(mod); // get data from slave device addr

    // return data from module
    return (uint8_t)data;
}

// I2C_WriteMultiple
// Write multiple bytes to a device.
// Param uint32_t "mod": base address of module
// Param uint8_t "addr": address to device
// Param uint8_t* "data": pointer to an array of bytes
// Param uint8_t "num_bytes": number of bytes to transmit
// Return: void
void I2C_WriteMultiple(uint32_t mod, uint8_t addr, uint8_t* data, uint8_t num_bytes) {

    I2CMasterSlaveAddrSet(mod, addr, false); // mod is reading from addr (False = write, True = read)
    I2CMasterDataPut(mod, *(data)); // write the byte into I2C data register
    I2CMasterControl(mod, I2C_MASTER_CMD_BURST_SEND_START); // start burst transaction of multiple bytes
    while(I2CMasterBusy(mod)); // wait for first byte to transmit

    data++;
    num_bytes--;

    while (num_bytes > 1) {
        I2CMasterDataPut(mod, *(data)); // place next byte into I2C data register
        I2CMasterControl(mod, I2C_MASTER_CMD_BURST_SEND_CONT);
        while(I2CMasterBusy(mod));

        data++;
        num_bytes--;
    }

    I2CMasterDataPut(mod, *(data));
    I2CMasterControl(mod, I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(mod));


    return;
}

// I2C_ReadMultiple
// Read multiple bytes from a device.
// Param uint32_t "mod": base address of module
// Param uint8_t "addr": address to device
// Param uint8_t* "data": pointer to an array of bytes
// Param uint8_t "num_bytes": number of bytes to read
// Return: void
void I2C_ReadMultiple(uint32_t mod, uint8_t addr, uint8_t* data, uint8_t num_bytes) {

    I2CMasterSlaveAddrSet(mod, addr, true);
    I2CMasterControl(mod, I2C_MASTER_CMD_BURST_RECEIVE_START); // start burst of reading multiple bytes



    while(I2CMasterBusy(mod)); // wait for first byte to transmit
    data[0] = I2CMasterDataGet(mod); // Store the first received byte.

    int i;

    for (i = 1; i < num_bytes; i++) {
        data[i] = I2CMasterDataGet(mod);

        if (i == num_bytes - 1) {
                   // Command the master to receive the last byte and stop.
                   I2CMasterControl(mod, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
               } else {
                   // Command the master to receive an intermediate byte and continue.
                   I2CMasterControl(mod, I2C_MASTER_CMD_BURST_RECEIVE_CONT);
               }

        while(I2CMasterBusy(mod)); // wait for module to be recieved

        data[i] = (uint8_t)I2CMasterDataGet(mod); // store result into data register from I2C register
    }


    return;
}

/********************************Public Functions***********************************/

