/*
 * extra_threads.h
 *
 *  Created on: Nov 17, 2025
 *      Author: steff
 */

#ifndef EXTRA_THREADS_H_
#define EXTRA_THREADS_H_

#include "./G8RTOS/G8RTOS_Semaphores.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>



void CamMove_Thread(void);
void Cube_Thread(void);
void Read_Buttons(void);
void Print_WorldCoords(void);



#endif /* EXTRA_THREADS_H_ */
