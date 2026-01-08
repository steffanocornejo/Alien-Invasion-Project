// multimod_BMI160.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for BMI160 functions

/************************************Includes***************************************/

#include "../multimod_BMI160.h"

#include <stdint.h>
#include "../multimod_i2c.h"

/************************************Includes***************************************/

/********************************Public Functions***********************************/

// BMI160_Init
// Initializes the BMI160. Currently enables the accelerometer
// in full-power mode.
// Return: void
void BMI160_Init() {
    I2C_Init(I2C_A_BASE);

    // Power on accelerometer
    // your code here

    BMI160_WriteRegister(BMI160_CMD_ADDR, 0x11); // enable accelerometer
    SysCtlDelay(30);
    BMI160_WriteRegister(BMI160_CMD_ADDR, 0x15); // enable gyroscope

    return;
}

// BMI160_Init
// Writes to a register address in the BMI160.
// Param uint8_t "addr": Register address
// Param uint8_t "data": data to write
// Return: void
void BMI160_WriteRegister(uint8_t addr, uint8_t data) {
    // write a single register
    // write where?
    uint8_t input[] = {addr, data};

    I2C_WriteMultiple(I2C_A_BASE, BMI160_ADDR, input, 2);
}

// BMI160_ReadRegister
// Reads from a register address in the BMI160.
// Param uint8_t "addr": Register address
// Return: void
uint8_t BMI160_ReadRegister(uint8_t addr) {
    // read from which addr?


    I2C_WriteSingle(I2C_A_BASE, BMI160_ADDR, addr); // writing the register value to BMI160_ADDR


    // return the data
    return (I2C_ReadSingle(I2C_A_BASE, BMI160_ADDR)); // reads the value of BMI160_ADDR
}

// BMI160_MultiReadRegister
// Uses the BMI160 auto-increment function to read from multiple registers.
// Param uint8_t "addr": beginning register address
// Param uint8_t* "data": pointer to an array to store data in
// Param uint8_t "num_bytes": number of bytes to read
// Return: void
void BMI160_MultiReadRegister(uint8_t addr, uint8_t* data, uint8_t num_bytes) {
    // write to which addr?

    // your code here
    I2C_WriteSingle(I2C_A_BASE, BMI160_ADDR, addr); // write to the address that we want to read from


    // read the data from that addr

    I2C_ReadMultiple(I2C_A_BASE, BMI160_ADDR, data, num_bytes); // read the bytes starting from that register


    return;
}

// BMI160_AccelXGetResult
// Gets the 16-bit x-axis acceleration result.
// Return: uint16_t
int16_t BMI160_AccelXGetResult() {

    uint8_t buf[2];
       int16_t result;

       // Wait until new accel data is ready (status register, bit 7)
//       while (!(BMI160_ReadRegister(BMI160_STATUS_ADDR) & (1 << 7)));

       // Burst read 2 bytes starting from ACC_X_L (0x12)
       BMI160_MultiReadRegister(0x12, buf, 2);

       // Combine little-endian LSB + MSB into signed 16-bit
       result = (int16_t)((buf[1] << 8) | buf[0]);

       return result;
}

// BMI160_AccelYGetResult
// Gets the 16-bit y-axis acceleration result.
// Return: uint16_t
int16_t BMI160_AccelYGetResult() {
    int16_t result = 0;
    // same as last

    return (result);
}

// BMI160_AccelZGetResult
// Gets the 16-bit z-axis acceleration result.
// Return: uint16_t
int16_t BMI160_AccelZGetResult() {
    int16_t result;

    return (result);
}

// BMI160_GyroXGetResult
// Gets the 16-bit x-axis gyroscope result.
// Return: uint16_t
int16_t BMI160_GyroXGetResult() {

    uint8_t buf[2];
    int16_t result;

          // Wait until new gyroscope data is ready.
          // According to the BMI160 datasheet, the drdy_gyr flag is bit 6
          // of the STATUS register (0x1B).
          while (!(BMI160_ReadRegister(0x1B) & (1 << 6)));

          // Burst read 2 bytes starting from GYR_X_L (0x0C).
          BMI160_MultiReadRegister(0x0C, buf, 2);

          // Combine the little-endian LSB and MSB into a signed 16-bit integer.
          // buf[0] is the LSB, buf[1] is the MSB.
          result = (int16_t)((buf[1] << 8) | buf[0]);

        return result;
}

// BMI160_GyroYGetResult
// Gets the 16-bit y-axis gyroscope result.
// Return: uint16_t
int16_t BMI160_GyroYGetResult() {
    int16_t result;
    // if not read, wait till read
        // your code here

    // read data
        // your code here

    // write data
        // your code here

    return (result);
}

// BMI160_GyroZGetResult
// Gets the 16-bit z-axis gyroscope result.
// Return: uint16_t
int16_t BMI160_GyroZGetResult() {
    int16_t result;
    // if not read, wait till read
        // your code here

    // read data
        // your code here

    // write data
        // your code here

    return (result);
}

// BMI160_AccelXYZGetResult
// Stores the 16-bit XYZ accelerometer results in an array.
// Param uint16_t* "data": pointer to an array of 16-bit data.
// Return: void
void BMI160_AccelXYZGetResult(uint16_t* data) {
    // if not read, wait till read
        // your code here

    // read data
        // your code here

    // write data 
        // your code here

    return;
}

// BMI160_GyroXYZGetResult
// Stores the 16-bit XYZ gyroscope results in an array.
// Param uint16_t* "data": pointer to an array of 16-bit data.
// Return: void
void BMI160_GyroXYZGetResult(uint16_t* data) {
    // if not read, wait till read
        // your code here

    // read data
        // your code here

    // write data 
        // your code here

    return;
}

// BMI160_GetDataStatus
// Gets the status register to determine if data is ready to read.
// Return: uint8_t
uint8_t BMI160_GetDataStatus() {
    // insert your code here
    uint8_t result;

    return result;
}

/********************************Public Functions***********************************/
