#ifndef __QUEUE_H 
#define __QUEUE_H

#include  <stdint.h>
#include  <stdbool.h>

#define FIFO_SIZE 2048

#define EMPTY 0
#define NOT_FULL 1
#define FULL 2

//implementation of fifo
typedef struct sFifo
{
	uint16_t Begin;
	uint16_t End;
	uint16_t *Data;
    uint16_t Size;
}tFifo;

void fifo_init(void);
void enqueue(uint16_t data );
uint16_t dequeue(void);
void FifoFlush(void);
bool IsFifoEmpty(void);
bool IsFifoFull(void);

#endif

