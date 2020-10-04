#ifndef _CPU_H_
#define _CPU_H_

#ifdef DEBUG
	#define PRINT_DBG(x, ...) printf(x, __VA_ARGS__)
#else
	#define PRINT_DBG(x, ...)
#endif

#define PUSH(PC) (*(--handle->SP) = (PC))
#define POP() (*(handle->SP++))

#include <stdint.h>

#include "rom.h"

// TODO: there must be a smarter way to do this

#define NOP     0x00
#define JR_N	0x18
#define LD_HLNN 0x21
#define LD_SP   0x31
#define SCF     0x37
#define LD_AI   0x3E
#define LD_AH	0x7C
#define LD_AL	0x7D
#define JP      0xC3
#define RET		0xC9
#define CALL	0xCD
#define LDH_NA  0xE0
#define PUSH_HL	0xE5
#define LD_NNA  0xEA
#define DI      0xF3	

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

void init_cpu(struct cpu* handle, struct rom* rom, uint8_t* ram);

uint8_t exec_instr(struct cpu* handle, struct rom* rom, uint8_t* ram);

#endif //_CPU_H_