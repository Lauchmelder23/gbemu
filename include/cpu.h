#ifndef _CPU_H_
#define _CPU_H_

#include <stdint.h>

#include "rom.h"

#define NOP 0x00
#define SCF 0x37
#define JP  0xC3

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
};

uint8_t exec_instr(struct cpu* handle, struct rom* rom, uint8_t* ram)
{
	handle->total_cycles++;
	if (--(handle->cycles) > 0)
		return;

	uint8_t opcode = *(handle->PC);
#ifdef DEBUG
	printf("0x%p %x ", handle->PC - rom->data, opcode);
#endif

	switch (opcode)
	{
	case NOP:
		handle->cycles = 4;
		handle->PC++;
#ifdef DEBUG
		printf("NOP");
#endif
		break;

	case SCF:
		handle->F.carry = 1;

		handle->cycles = 4;
		handle->PC++;
#ifdef DEBUG
		printf("NOP");
#endif
		break;

	case JP:
	{
		uint8_t lo_byte = *(handle->PC + 1);
		uint8_t hi_byte = *(handle->PC + 2);
		uint16_t jp_addr = (hi_byte << 8) | (lo_byte);

		handle->cycles = 12;
		handle->PC = rom->data + jp_addr;
#ifdef DEBUG
		printf("%x %x JP 0x%x", lo_byte, hi_byte, jp_addr);
#endif
	} break;

	default:
		fprintf(stderr, "Unknown opcode: 0x%x\n", opcode);
		return 0;
	}

#ifdef DEBUG
	printf("\t\t CYC: %u\n", handle->total_cycles);
#endif

	return 1;
}

#endif _CPU_H_