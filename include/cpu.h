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
#define LD_BCNN	0x01
#define INC_BC	0x03
#define DEC_B	0x05
#define LD_BN	0x06
#define JR_N	0x18
#define LD_HLNN 0x21
#define INC_HL	0x23
#define JR_NZ	0x20
#define JR_Z	0x28
#define LDI_AHL 0x2A
#define JR_NC	0x30
#define LD_SP   0x31
#define LDD_HLA 0x32
#define SCF     0x37
#define JR_C	0x38
#define INC_A	0x3C
#define LD_AI   0x3E
#define LD_HB	0x60
#define LD_HHL	0x66
#define LD_AB	0x78
#define LD_AH	0x7C
#define LD_AL	0x7D
#define LD_AHL	0x7E
#define OR_C	0xB1
#define POP_BC	0xC1
#define JP      0xC3
#define CALL_NZ 0xC4
#define PUSH_BC 0xC5
#define RET		0xC9
#define CALL	0xCD
#define LDH_NA  0xE0
#define POP_HL  0xE1
#define PUSH_HL	0xE5
#define AND_IMM 0xE6
#define LD_NNA  0xEA
#define LDH_AN	0xF0
#define POP_AF	0xF1
#define DI      0xF3
#define PUSH_AF 0xF5
#define LD_ANN	0xFA
#define CP_IMM	0xFE

extern const uint8_t BOOTLOADER[256];

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

uint8_t reset_cpu(struct cpu* handle, struct rom* rom, uint8_t* ram);

uint8_t exec_instr(struct cpu* handle, struct rom* rom, uint8_t* ram);

#endif //_CPU_H_