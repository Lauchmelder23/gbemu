#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "rom.h"

int main(int argc, char** argv)
{
	uint8_t* ram = (uint8_t*)malloc(UINT16_MAX);
	if (ram == NULL)
	{
		fprintf(stderr, "Failed to allocate memory for RAM (%u Bytes)\n", UINT16_MAX);
		return -1;
	}

	struct rom myRom;
	readROM("roms/cpu_instrs.gb", &myRom, ram);
	if (myRom.data == NULL)
	{
		free(ram);
		return -1;
	}

	struct cpu myCpu;
	myCpu.cycles = 1;
	myCpu.total_cycles = 0;
	myCpu.PC = myRom.entrypoint;

	while (exec_instr(&myCpu, &myRom, ram));

	free(ram);
	return 0;
}