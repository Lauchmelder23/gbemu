#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "gpu.h"
#include "rom.h"

int main(int argc, char** argv)
{
	uint8_t* ram = (uint8_t*)malloc(UINT16_MAX + 1);
	if (ram == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for RAM (%u Bytes)\n", UINT16_MAX + 1);
		return -1;
	}

	struct rom myRom;
	readROM("roms/cpu_instrs.gb", &myRom, ram);
	if (myRom.data == NULL)
	{
		free(ram);
		return -1;
	}

	struct gpu myGpu;
	if (!init_gpu(&myGpu, ram))
	{
		free(ram);
		return -1;
	}

	struct cpu myCpu;
	if (!reset_cpu(&myCpu, &myGpu, &myRom, ram)) 
	{
		free(ram);
		return -1;
	}

	return 0;

	while (exec_instr(&myCpu, &myGpu, &myRom, ram))
	{
		tick_gpu(&myGpu);
	}

	free(ram);
	return 0;
}