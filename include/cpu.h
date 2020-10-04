#ifndef _CPU_H_
#define _CPU_H_

#ifdef DEBUG
	#define PRINT_DBG(x, ...) printf(x, __VA_ARGS__);
#else
	#define PRINT_DBG(x, ...)
#endif

#include <stdint.h>

#include "rom.h"

#define NOP    0x00
#define LD_SP  0x31
#define SCF    0x37
#define JP     0xC3
#define LD_NNA 0xEA
#define DI     0xF3	

union flag_register
{
	struct
	{
		uint8_t unused_1 : 1;
		uint8_t unused_2 : 1;
		uint8_t unused_3 : 1;
		uint8_t unused_4 : 1;
		uint8_t carry : 1;
		uint8_t half_carry : 1;
		uint8_t negative : 1;	
		uint8_t zero : 1;
	};

	uint8_t val;
};

struct cpu
{
	union {
		struct {
			union flag_register F;
			uint8_t A;
		};

		uint16_t AF;
	};

	union {
		struct {
			uint8_t C;
			uint8_t B;
		};

		uint16_t BC;
	};

	union {
		struct {
			uint8_t E;
			uint8_t D;
		};

		uint16_t DE;
	};

	union {
		struct {
			uint8_t L;
			uint8_t H;
		};

		uint16_t HL;
	};

	uint8_t* PC;
	uint8_t* SP;
	uint64_t total_cycles;
	uint8_t cycles;

	int8_t interrupt;
};

void init_cpu(struct cpu* handle, struct rom* rom, uint8_t* ram)
{
	handle->cycles = 1;
	handle->total_cycles = 0;
	handle->PC = rom->entrypoint;
	handle->SP = ram;
	handle->AF = 0x0000;
	handle->BC = 0x0000;
	handle->DE = 0x0000;
	handle->HL = 0x0000;
}

uint8_t exec_instr(struct cpu* handle, struct rom* rom, uint8_t* ram)
{
	handle->total_cycles++;
	if (--(handle->cycles) > 0)
		return 1;

	int8_t interruptSet = handle->interrupt;

	uint8_t opcode = *(handle->PC);
	PRINT_DBG("$%04X %02X ", handle->PC - rom->data, opcode);

	switch (opcode)
	{
	case NOP:
		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c %-20s", 5, ' ', "NOP");
		break;

	case LD_SP:
		handle->SP = ram + (((uint16_t)(*(handle->PC + 2)) << 8) | (*(handle->PC + 1)));

		handle->cycles = 12;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD SP %*c", *(handle->PC - 2), *(handle->PC - 1), 14, ' ');
		break;

	case SCF:
		handle->F.carry = 1;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c NOP %*c", 5, ' ', 20, ' ');
		break;

	case JP:
	{
		uint8_t lo_byte = *(handle->PC + 1);
		uint8_t hi_byte = *(handle->PC + 2);
		uint16_t jp_addr = (hi_byte << 8) | (lo_byte);

		handle->cycles = 12;
		handle->PC = rom->data + jp_addr;

		PRINT_DBG("%02X %02X JP $%04X %*c", lo_byte, hi_byte, jp_addr, 11, ' ');
	} break;

	case LD_NNA:
	{
		uint16_t addr = ((uint16_t)(*handle->PC + 2) << 8) | *(handle->PC + 1);
		*(ram + addr) = handle->A;
		
		handle->cycles = 16;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD ($%04X), A %*c", *(handle->PC + 1), *(handle->PC + 2), addr, 6, ' ');
	} break;

	case DI:
		handle->interrupt = -1;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c DI %*c", 5, ' ', 17, ' ');
		break;

	default:
		fprintf(stderr, "Unknown opcode.");
		return 0;
	}


	if (interruptSet)
	{
		*(ram + 0xFFFF) = (interruptSet == 1) ? (uint8_t)1 : (uint8_t)0;
		handle->interrupt = 0;
	}


	PRINT_DBG("SP: %04X I: %02X CYC: %u\n", handle->SP - ram, *(ram + 0xFFFF), handle->total_cycles);

	return 1;
}

#endif //_CPU_H_