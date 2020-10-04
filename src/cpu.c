#include "cpu.h"
#include "rom.h"

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
	PRINT_DBG("$%04X %02X ", (uint16_t)(handle->PC - rom->data), opcode);

	switch (opcode)
	{
	case NOP:
	{
		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c %-20s", 5, ' ', "NOP");
	} break;

	case JR_N:
	{
		uint8_t offset = *(handle->PC + 1);

		PRINT_DBG("%02X %*c JR 0x%02X %*c", offset, 2, ' ', offset, 12, ' ');

		handle->cycles = 8;
		handle->PC += offset;
	} break;

	case LD_HLNN:
	{
		uint16_t val = ((uint16_t)(*(handle->PC + 2)) << 8) | *(handle->PC + 1);
		handle->HL = val;

		handle->cycles = 12;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD HL, 0x%04X %*c", *(handle->PC - 2), *(handle->PC - 1), val, 6, ' ');
	} break;

	case LD_SP:
	{
		handle->SP = ram + (((uint16_t)(*(handle->PC + 2)) << 8) | (*(handle->PC + 1)));

		handle->cycles = 12;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD SP, 0x%04X %*c", *(handle->PC - 2), *(handle->PC - 1), (uint16_t)(handle->SP - ram), 6, ' ');
	} break;

	case SCF:
	{
		handle->F.carry = 1;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c NOP %*c", 5, ' ', 20, ' ');
	} break;

	case LD_AI:
	{
		handle->A = *(handle->PC + 1);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LD A, 0x%02X %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 9, ' ');
	} break;

	case LD_AH:
	{
		handle->A = handle->H;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD A, H %*c", 5, ' ', 12, ' ');
	} break;

	case LD_AL:
	{
		handle->A = handle->L;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD A, L %*c", 5, ' ', 12, ' ');
	} break;

	case JP:
	{
		uint8_t lo_byte = *(handle->PC + 1);
		uint8_t hi_byte = *(handle->PC + 2);
		uint16_t jp_addr = (hi_byte << 8) | (lo_byte);

		handle->cycles = 12;
		handle->PC = rom->data + jp_addr;

		PRINT_DBG("%02X %02X JP $%04X %*c", lo_byte, hi_byte, jp_addr, 11, ' ');
	} break;

	case RET:
	{
		uint8_t hi = POP();
		uint8_t lo = POP();
		handle->PC = (rom->data + (((uint16_t)hi << 8) | lo));

		handle->cycles = 8;

		PRINT_DBG("%*c RET $%04X %*c", 5, ' ', (uint16_t)(handle->PC - rom->data), 10, ' ');
	} break;

	case CALL:
	{
		uint16_t addr = ((uint16_t)(*(handle->PC + 2)) << 8) | *(handle->PC + 1);

		PUSH((uint8_t)((handle->PC - rom->data + 3) & 0x00FF));
		PUSH((uint8_t)((handle->PC - rom->data + 3) >> 8));

		PRINT_DBG("%02X %02X CALL $%04X %*c", *(handle->PC + 1), *(handle->PC + 2), addr, 9, ' ');

		handle->cycles = 12;
		handle->PC = rom->data + addr;
	} break;

	case LDH_NA:
	{
		*(ram + 0xFF00 + *(handle->PC + 1)) = handle->A;

		handle->cycles = 12;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LDH ($FF00+%02X), A %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 2, ' ');
	} break;

	case PUSH_HL:
	{
		PUSH(handle->L);
		PUSH(handle->H);

		handle->cycles = 16;
		handle->PC++;

		PRINT_DBG("%*c PUSH HL %*c", 5, ' ', 12, ' ');

	} break;

	case LD_NNA:
	{
		uint16_t addr = ((uint16_t)(*(handle->PC + 2)) << 8) | *(handle->PC + 1);
		*(ram + addr) = handle->A;

		handle->cycles = 16;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD ($%04X), A %*c", *(handle->PC - 2), *(handle->PC - 1), addr, 6, ' ');
	} break;

	case DI:
	{
		handle->interrupt = -1;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c DI %*c", 5, ' ', 17, ' ');
	} break;

	default:
		fprintf(stderr, "Unknown opcode.");
		return 0;
	}


	if (interruptSet)
	{
		*(ram + 0xFFFF) = (interruptSet == 1) ? (uint8_t)1 : (uint8_t)0;
		handle->interrupt = 0;
	}


	PRINT_DBG("AF: %04X BC: %04X DE: %04X, HL: %04X SP: %04X I: %02X CYC: %I64u\n", handle->AF, handle->BC, handle->DE, handle->HL, (uint16_t)(handle->SP - ram), *(ram + 0xFFFF), handle->total_cycles);

	return 1;
}