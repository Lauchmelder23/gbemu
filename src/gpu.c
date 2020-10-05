#include "gpu.h"

init_gpu(struct gpu* gpu, uint8_t* ram)
{
	gpu->cycle = 1;
	gpu->curline = ram + 0xFF44;
	*(gpu->curline) = 0x00;
}

tick_gpu(struct gpu* gpu, uint8_t n)
{
	for (uint8_t i = 0; i < n; i++)
	{
		gpu->cycle++;

		if (gpu->cycle % 456 == 0)
			(*(gpu->curline))++;

		if(gpu->cycle == 70224)
			gpu->cycle = 1;
	}
}
