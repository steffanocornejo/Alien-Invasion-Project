/*
 * extra_threads.c
 *
 *  Created on: Nov 10, 2025
 *      Author: steff
 */





// * OLD THREAD CODE *






//void Read_Accel_Periodic(void) { // producer thread
//
//        G8RTOS_WaitSemaphore(&sem_I2CA);
//        int16_t x_accel_value = BMI160_AccelXGetResult();
//        G8RTOS_SignalSemaphore(&sem_I2CA);
//
////        float p_width = (float)abs(x_accel_value) / 18384.0f;
////        LaunchpadLED_PWMSetDuty(BLUE, p_width);
//
//        G8RTOS_WriteFIFO(0, x_accel_value);
//
//
////        G8RTOS_WaitSemaphore(&sem_UART);
////        UARTprintf("Printed Blue\n");
////        G8RTOS_SignalSemaphore(&sem_UART);
//
//
//
//}
//
//// Thread1, reads gyro_x data, adjusts RED led duty cycle.
//void Thread1(void) {
//
//    while (1) {
//
////        G8RTOS_WaitSemaphore(&sem_I2CA);
////        int16_t gyro_value = BMI160_GyroXGetResult();
////        G8RTOS_SignalSemaphore(&sem_I2CA);
//
////        float g_width = (float)abs(gyro_value) / 8000.0f;
////
////        LaunchpadLED_PWMSetDuty(RED, g_width);
//
//
//       int32_t dataFromFIFO = G8RTOS_ReadFIFO(0);
//
//        G8RTOS_WaitSemaphore(&sem_UART);
//        UARTprintf("Accelerometer Value from Thread0: %d\n", dataFromFIFO);
//        G8RTOS_SignalSemaphore(&sem_UART);
//
//
//        sleep(10);
//    }
//}
//
//// Thread2, reads optical sensor values, adjusts GREEN led duty cycle.
//void Thread2(void) {
//
//    while (1) {
//
//        G8RTOS_WaitSemaphore(&sem_I2CA);
//
//        uint16_t opt_value = OPT3001_GetResult();
//        G8RTOS_SignalSemaphore(&sem_I2CA);
//
//        float o_width = (float)abs(opt_value) / 1000.0f;
//
//        LaunchpadLED_PWMSetDuty(GREEN, o_width);
//
//        sleep(10);
//
//    }
//
//}
//
//// Thread3, reads and output button 1 status using polling
//void Thread3(void) {
//    while(1){
//
//
//           // access sensor!
//
//           if (LaunchpadButtons_ReadSW1()) {
//               G8RTOS_WaitSemaphore(&sem_UART);
//
//               UARTprintf("Switch 1 Pressed!\n");
//
//               G8RTOS_SignalSemaphore(&sem_UART);
//           }
//           sleep(10);
//       }
//
//}
//
//
//// Thread4, reads and output button 2 status using polling
//void Thread4(void) {
//    while(1){
//
//
//        // access sensor!
//
//        if (LaunchpadButtons_ReadSW2()) {
//            G8RTOS_WaitSemaphore(&sem_UART);
//
//            UARTprintf("Switch 2 Pressed!\n");
//
//            G8RTOS_SignalSemaphore(&sem_UART);
//        }
//
//
//         sleep(10);
//
//        }
//
//}


