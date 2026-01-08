// Lab 3, uP2 Fall 2025
// Space Invaders Game
// Main initialization and RTOS launch

/************************************Includes***************************************/

#include "G8RTOS/G8RTOS.h"
#include "./MultimodDrivers/multimod.h"
#include "driverlib/fpu.h"
#include "./threads.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <inc/tm4c123gh6pm.h>
#include <inc/hw_memmap.h>
#include <inc/hw_gpio.h>

/************************************Includes***************************************/

/********************************Public Variables***********************************/
extern uint32_t SystemTime;

/********************************Public Variables***********************************/

/************************************MAIN*******************************************/

int main(void) {

    // Enable FPU (optional but recommended for floating point operations)
    FPUEnable();
 //   FPULazyStackingDisable();

    // Set system clock to 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

    // Short delay for clock to stabilize
    SysCtlDelay(16000000 / 10); // ~100ms delay

    // Initialize random number generator with time
    srand(SystemTime);

    // Initialize G8RTOS
    G8RTOS_Init();

    // Initialize multimod board peripherals
    Multimod_Init();

    // Initialize UART for debugging
    // UART_Init();  // UART0 for debugging via PC (uncomment if needed)

    // Clear screen to black
    ST7789_Fill(~ST7789_BLACK);

    // ===========================
    // Initialize Semaphores
    // ===========================
    G8RTOS_InitSemaphore(&sem_I2CA, 1);           // I2C (accelerometer, buttons)
    G8RTOS_InitSemaphore(&sem_SPIA, 1);           // SPI (display)
    G8RTOS_InitSemaphore(&sem_UART, 1);           // UART debugging
    G8RTOS_InitSemaphore(&sem_gameState, 1);      // Game state protection
    G8RTOS_InitSemaphore(&sem_Bullet, 0);         // Bullet creation signal (starts at 0)
    G8RTOS_InitSemaphore(&sem_Mystery, 0);         // Bullet creation signal (starts at 0)
    G8RTOS_InitSemaphore(&sem_ScoreChange, 0);    // Score update signal (starts at 0)
    G8RTOS_InitSemaphore(&sem_PCA9555_Debounce, 0); // Button interrupt debounce
    G8RTOS_InitSemaphore(&sem_gameOver, 0); // Button interrupt debounce

    // ===========================
    // Initialize FIFOs
    // ===========================
    G8RTOS_InitFIFO(JOYSTICK_FIFO);   // Joystick data

    // ===========================
    // Initialize Game State (NO drawing yet - RTOS not running)
    // ===========================
    Init_Game();

    // ===========================
    // Add Background Threads
    // ===========================

    G8RTOS_AddThread(Idle_Thread, 254, "idle");
    G8RTOS_AddThread(Player_Thread, 2, "player");
    G8RTOS_AddThread(Board_Thread, 3, "board");
    G8RTOS_AddThread(Read_Buttons_Thread, 4, "buttons");
    G8RTOS_AddThread(Create_Bullet_Thread, 5, "bullet");
    G8RTOS_AddThread(Spawn_Asteroids_Thread, 6, "Spawn");
    G8RTOS_AddThread(Game_Over_Thread, 7, "gg");
    G8RTOS_AddThread(Mystery_Box_Thread, 8, "mystery");

    // ===========================
    // Add Periodic Threads
    // ===========================
    G8RTOS_Add_PeriodicEvent(Read_Joystick, 5, 1);       // Every 5ms - read joystick

    // ===========================
    // Add Aperiodic Threads (Interrupts)
    // ===========================
    G8RTOS_Add_APeriodicEvent(Button_Handler, 1, 20);      // Button interrupt handler

    // ===========================
    // Launch RTOS
    // ===========================
    UARTprintf("Launching RTOS...\n");
    G8RTOS_Launch();

    // Should never reach here - RTOS takes over
    while (1);
}

/************************************MAIN*******************************/
