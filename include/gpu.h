#ifndef _GPU_H_
#define _GPU_H_

#include <stdint.h>

struct gpu
{
	uint8_t* curline;

	uint64_t cycle;
};

uint8_t init_gpu(struct gpu* gpu, uint8_t* ram);

uint8_t tick_gpu(struct	gpu* gpu, uint8_t n);

#endif //_GPU_H_