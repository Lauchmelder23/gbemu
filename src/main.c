#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
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
	readROM("roms/cpu_instrs.gb", &myRom);
	if (myRom.data == NULL)
	{
		free(ram);
		return -1;
	}

	struct cpu myCpu;
	reset_cpu(&myCpu, &myRom, ram);

	while (exec_instr(&myCpu, &myRom, ram));

	free(ram);
	return 0;
}