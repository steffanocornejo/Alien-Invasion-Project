// G8RTOS_IPC.c
// Date Created: 2023-07-25
// Date Updated: 2023-07-27
// Defines for FIFO functions for interprocess communication

#include "../G8RTOS_IPC.h"

/************************************Includes***************************************/

#include "../G8RTOS_Semaphores.h"

/************************************Includes***************************************/

/******************************Data Type Definitions********************************/

#define FIFO_SIZE 16
#define MAX_NUMBER_OF_FIFOS 4

/******************************Data Type Definitions********************************/

/****************************Data Structure Definitions*****************************/

typedef struct G8RTOS_FIFO_t {
    int32_t buffer[FIFO_SIZE];
    int32_t *head;
    int32_t *tail;
    uint32_t lostData;
    semaphore_t current_size;
    uint32_t count;
    semaphore_t mutex;


} G8RTOS_FIFO_t;

/****************************Data Structure Definitions*****************************/

/***********************************Externs*****************************************/
/***********************************Externs*****************************************/

/********************************Public Variables***********************************/

static G8RTOS_FIFO_t FIFOs[MAX_NUMBER_OF_FIFOS];

/********************************Public Variables***********************************/

/********************************Public Functions***********************************/

// G8RTOS_InitFIFO
// Initializes FIFO, points head & tai to relevant buffer
// memory addresses.
// Param "FIFO_index": Index of FIFO block
// Return: int32_t, -1 if error (i.e. FIFO full), 0 if okay
int32_t G8RTOS_InitFIFO(uint32_t FIFO_index) {
    if (FIFO_index >= MAX_NUMBER_OF_FIFOS) {
           return -1; // invalid index
       }

    G8RTOS_FIFO_t *fifo = &FIFOs[FIFO_index];

    fifo->head = fifo->buffer;
    fifo->tail = fifo->buffer;
    fifo->count = 0;
    fifo->lostData = 0;

    // Initialize semaphores
    G8RTOS_InitSemaphore(&fifo->current_size, 0);
    G8RTOS_InitSemaphore(&fifo->mutex, 1);

    return 0; // success


}

// G8RTOS_ReadFIFO
// Reads data from head pointer of FIFO.
// Param "FIFO_index": Index of FIFO block
// Return: int32_t, data at head pointer
int32_t G8RTOS_ReadFIFO(uint32_t FIFO_index) {
    if (FIFO_index >= MAX_NUMBER_OF_FIFOS) {
           return -1;
       }

       G8RTOS_FIFO_t *fifo = &FIFOs[FIFO_index];
       int32_t data;

       G8RTOS_WaitSemaphore(&fifo->current_size); // wait until theres at least something in the FIFO
       G8RTOS_WaitSemaphore(&fifo->mutex); // once something detected, lock to read

       data = *(fifo->head);           // read data from FIFO
       fifo->head++;                   // move head forward

       if (fifo->head == fifo->buffer + FIFO_SIZE) {
          fifo->head = fifo->buffer;  // wrap head pointer
       }

       fifo->count--; // we read an item from the FIFO

       G8RTOS_SignalSemaphore(&fifo->mutex); // unlock so others can read/write

       return data;

}

// G8RTOS_WriteFIFO
// Writes data to tail of buffer.
// Param "FIFO_index": Index of FIFO block
// Return: int32_t, data at head pointer
int32_t G8RTOS_WriteFIFO(uint32_t FIFO_index, uint32_t data) {
    if (FIFO_index >= MAX_NUMBER_OF_FIFOS)
            return -1;

        G8RTOS_FIFO_t *fifo = &FIFOs[FIFO_index];

        G8RTOS_WaitSemaphore(&fifo->mutex); // lock so that producer can write

        // Check if FIFO is full (CurrentSize == FIFO_SIZE)

        if (fifo->count == FIFO_SIZE) {
            fifo->lostData++; // count overflow
            fifo->head++;    // Overwrite oldest data
            if (fifo->head == fifo->buffer + FIFO_SIZE) {
                fifo->head = fifo->buffer;
            }
        }
        else {
            fifo->count++;
        }

        *(fifo->tail) = data;
        fifo->tail++;

        if (fifo->tail == fifo->buffer + FIFO_SIZE)
            fifo->tail = fifo->buffer;

        G8RTOS_SignalSemaphore(&fifo->current_size);   // Signal to consumer that there is now one more item
        G8RTOS_SignalSemaphore(&fifo->mutex); // unlock the read/write


        return 0;
}

/********************************Public Functions***********************************/
