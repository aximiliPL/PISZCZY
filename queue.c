#include "queue.h"

struct sFifo* f;
uint16_t buf[FIFO_SIZE];

static uint16_t FifoNext( uint16_t index )
{
	return ( index + 1 ) % f->Size;
}

void fifo_init(void)
{
	f->Begin = 0;
	f->End = 0;
	f->Data = buf;
	f->Size = FIFO_SIZE;
}

void enqueue( uint16_t data )
{
	f->End = FifoNext( f->End );
	f->Data[f->End] = data;
}

uint16_t dequeue(void)
{
	uint16_t data = f->Data[FifoNext( f->Begin )];

	f->Begin = FifoNext( f->Begin );
	return data;
}

void FifoFlush(void)
{
	f->Begin = 0;
	f->End = 0;
}

bool IsFifoEmpty(void)
{
	return ( f->Begin == f->End );
}

bool IsFifoFull(void)
{
	return ( FifoNext( f->End ) == f->Begin );
}

