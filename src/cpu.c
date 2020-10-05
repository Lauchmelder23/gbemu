#include "cpu.h"
#include "rom.h"

const uint8_t BOOTLOADER[256] = {
	0x31, 0xFE, 0xFF, 0xAF, 0x21, 0xFF, 0x9F, 0x32,
	0xCB, 0x7C, 0x20, 0xFB, 0x21, 0x26, 0xFF, 0x0E,
	0x11, 0x3E, 0x80, 0x32, 0xE2, 0x0C, 0x3E, 0xF3,
	0xE2, 0x32, 0x3E, 0x77, 0x77, 0x3E, 0xFC, 0xE0,
	0x47, 0x11, 0x04, 0x01, 0x21, 0x10, 0x80, 0x1A,
	0xCD, 0x95, 0x00, 0xCD, 0x96, 0x00, 0x13, 0x7B,
	0xFE, 0x34, 0x20, 0xF3, 0x11, 0xD8, 0x00, 0x06,
	0x08, 0x1A, 0x13, 0x22, 0x23, 0x05, 0x20, 0xF9,
	0x3E, 0x19, 0xEA, 0x10, 0x99, 0x21, 0x2F, 0x99,
	0x0E, 0x0C, 0x3D, 0x28, 0x08, 0x32, 0x0D, 0x20,
	0xF9, 0x2E, 0x0F, 0x18, 0xF3, 0x67, 0x3E, 0x64,
	0x57, 0xE0, 0x42, 0x3E, 0x91, 0xE0, 0x40, 0x04,
	0x1E, 0x02, 0x0E, 0x0C, 0xF0, 0x44, 0xFE, 0x90,
	0x20, 0xFA, 0x0D, 0x20, 0xF7, 0x1D, 0x20, 0xF2,
	0x0E, 0x13, 0x24, 0x7C, 0x1E, 0x83, 0xFE, 0x62,
	0x28, 0x06, 0x1E, 0xC1, 0xFE, 0x64, 0x20, 0x06,
	0x7B, 0xE2, 0x0C, 0x3E, 0x87, 0xE2, 0xF0, 0x42,
	0x90, 0xE0, 0x42, 0x15, 0x20, 0xD2, 0x05, 0x20,
	0x4F, 0x16, 0x20, 0x18, 0xCB, 0x4F, 0x06, 0x04,
	0xC5, 0xCB, 0x11, 0x17, 0xC1, 0xCB, 0x11, 0x17,
	0x05, 0x20, 0xF5, 0x22, 0x23, 0x22, 0x23, 0xC9,
	0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B,
	0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
	0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E,
	0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
	0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC,
	0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
	0x3C, 0x42, 0xB9, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,
	0x21, 0x04, 0x01, 0x11, 0xA8, 0x00, 0x1A, 0x13,
	0xBE, 0x20, 0xFE, 0x23, 0x7D, 0xFE, 0x34, 0x20,
	0xF5, 0x06, 0x19, 0x78, 0x86, 0x23, 0x05, 0x20,
	0xFB, 0x86, 0x20, 0xFE, 0x3E, 0x01, 0xE0, 0x50
};

uint8_t reset_cpu(struct cpu* handle, struct rom* rom, uint8_t* ram)
{
	handle->cycles = 0;
	handle->total_cycles = 0;
	handle->PC = BOOTLOADER;

	while (handle->PC >= BOOTLOADER && handle->PC < BOOTLOADER + 256)
	{
		if (!exec_instr(handle, rom, ram)) return 0;
	}

	handle->PC = rom->data + 0x100;
	return 1;
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

	case LD_BCNN:
	{
		uint16_t val = ((uint16_t)(*(handle->PC + 2)) << 8) | *(handle->PC + 1);
		handle->BC = val;

		handle->cycles = 12;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD BC, 0x%04X %*c", *(handle->PC - 2), *(handle->PC - 1), val, 6, ' ');
	} break;

	case INC_BC:
	{
		handle->BC++;

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c INC BC %*c", 5, ' ', 13, ' ');
	} break;

	case DEC_B:
	{
		uint16_t tmp = handle->B - 1;

		handle->F.zero = (handle->B == 0);
		handle->F.negative = 1;
		handle->F.half_carry = ((tmp & 0x10) != (handle->B & 0x10));
		handle->F.carry = (tmp & 0x100);

		handle->B = tmp;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c DEC B %*c", 5, ' ', 14, ' ');
	} break;

	case LD_BN:
	{
		handle->B = *(handle->PC + 1);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LD B, 0x%02X %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 9, ' ');
	} break;

	case JR_N:
	{
		int8_t offset = *(handle->PC + 1);

		PRINT_DBG("%02X %*c JR 0x%02X %*c", (uint8_t)offset, 2, ' ', (uint8_t)offset, 12, ' ');

		handle->cycles = 8;
		handle->PC += 2;
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

	case INC_HL:
	{
		handle->HL++;

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c INC HL %*c", 5, ' ', 13, ' ');
	} break;

	case JR_NZ:
	{
		int8_t offset = *(handle->PC + 1);

		PRINT_DBG("%02X %*c JR NZ 0x%02X %*c", (uint8_t)offset, 2, ' ', (uint8_t)offset, 9, ' ');

		if (!handle->F.zero)
			handle->PC += offset;

		handle->cycles = 8;
		handle->PC += 2;
	} break;

	case JR_Z:
	{
		int8_t offset = *(handle->PC + 1);
		
		PRINT_DBG("%02X %*c JR Z 0x%02X %*c", (uint8_t)offset, 2, ' ', (uint8_t)offset, 10, ' ');
		
		if (handle->F.zero)
			handle->PC += offset;

		handle->cycles = 8;
		handle->PC += 2;
	} break;

	case LDI_AHL:
	{
		handle->A = *(ram + handle->HL++);

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c LDI A, (HL) %*c", 5, ' ', 8, ' ');
	} break;

	case JR_NC:
	{
		int8_t offset = *(handle->PC + 1);

		PRINT_DBG("%02X %*c JR NC 0x%02X %*c", (uint8_t)offset, 2, ' ', (uint8_t)offset, 9, ' ');

		if (!handle->F.carry)
			handle->PC += offset;

		handle->cycles = 8;
		handle->PC += 2;
	} break;

	case LD_SP:
	{
		handle->SP = ram + (((uint16_t)(*(handle->PC + 2)) << 8) | (*(handle->PC + 1)));

		handle->cycles = 12;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD SP, 0x%04X %*c", *(handle->PC - 2), *(handle->PC - 1), (uint16_t)(handle->SP - ram), 6, ' ');
	} break;

	case LDD_HLA:
	{
		*(ram + handle->HL) = handle->A;
		handle->HL--;

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c LDD (HL), A %*c", 5, ' ', 8, ' ');
	} break;

	case SCF:
	{
		handle->F.carry = 1;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c NOP %*c", 5, ' ', 20, ' ');
	} break;

	case JR_C:
	{
		uint8_t offset = *(handle->PC + 1);

		PRINT_DBG("%02X %*c JR C 0x%02X %*c", offset, 2, ' ', offset, 10, ' ');

		if (handle->F.carry)
			handle->PC += offset;

		handle->cycles = 8;
		handle->PC += 2;
	} break;

	case INC_A:
	{
		uint8_t tmp = handle->A + 1;

		handle->F.zero = (handle->A == 0);
		handle->F.negative = 1;
		handle->F.half_carry = ((tmp & 0x10) != (handle->A & 0x10));

		handle->A = tmp;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c INC A %*c", 5, ' ', 14, ' ');
	} break;

	case LD_AI:
	{
		handle->A = *(handle->PC + 1);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LD A, 0x%02X %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 9, ' ');
	} break;

	case LD_HB:
	{
		handle->H = handle->B;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD H, B %*c", 5, ' ', 12, ' ');
	} break;

	case LD_HHL:
	{
		handle->H = *(ram + handle->HL);

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c LD H, (HL) %*c", 5, ' ', 9, ' ');
	} break;

	case LD_AB:
	{
		handle->A = handle->B;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD A, B %*c", 5, ' ', 12, ' ');

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

	case LD_AHL:
	{
		handle->A = *(ram + handle->HL);

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c LD A, ($%04X) %*c", 5, ' ', handle->HL, 6, ' ');
	} break;

	case OR_C:
	{
		handle->A |= handle->C;

		handle->F.val = 0x00;
		handle->F.zero = (handle->A == 0);

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c OR C %*c", 5, ' ', 15, ' ');
	} break;

	case POP_BC:
	{
		handle->B = POP();
		handle->C = POP();

		handle->cycles = 12;
		handle->PC++;

		PRINT_DBG("%*c POP BC %*c", 5, ' ', 13, ' ');
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

	case CALL_NZ:
	{
		uint16_t addr = (*(handle->PC + 2) << 8) | *(handle->PC + 1);

		PRINT_DBG("%02X %02X CALL NZ $%04X %*c", *(handle->PC + 1), *(handle->PC + 2), addr, 6, ' ');

		if (handle->F.zero == 0)
			handle->PC = rom->data + addr;
		else
			handle->PC += 3;

		handle->cycles = 12;
	} break;

	case PUSH_BC:
	{
		PUSH(handle->C);
		PUSH(handle->B);

		handle->cycles = 16;
		handle->PC++;

		PRINT_DBG("%*c PUSH BC %*c", 5, ' ', 12, ' ');
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

	case POP_HL:
	{
		handle->H = POP();
		handle->L = POP();

		handle->cycles = 12;
		handle->PC++;

		PRINT_DBG("%*c POP HL %*c", 5, ' ', 13, ' ');
	} break;

	case PUSH_HL:
	{
		PUSH(handle->L);
		PUSH(handle->H);

		handle->cycles = 16;
		handle->PC++;

		PRINT_DBG("%*c PUSH HL %*c", 5, ' ', 12, ' ');
	} break;

	case AND_IMM:
	{
		handle->A &= *(handle->PC + 1);

		handle->F.val = 0x00;
		handle->F.zero = (handle->A == 0x00);
		handle->F.half_carry = 0x01;

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%02X %*c AND A, 0x%02X %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 8, ' ');
	} break;

	case LD_NNA:
	{
		uint16_t addr = ((uint16_t)(*(handle->PC + 2)) << 8) | *(handle->PC + 1);
		*(ram + addr) = handle->A;

		handle->cycles = 16;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD ($%04X), A %*c", *(handle->PC - 2), *(handle->PC - 1), addr, 6, ' ');
	} break;

	case LDH_AN:
	{
		handle->A = *(ram + 0xFF00 + *(handle->PC + 1));

		handle->cycles = 12;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LDH A, ($FF00+%02X) %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 2, ' ');
	} break;

	case POP_AF:
	{
		handle->A = POP();
		handle->F.val = POP();

		handle->cycles = 12;
		handle->PC++;

		PRINT_DBG("%*c POP AF %*c", 5, ' ', 13, ' ');
	} break;

	case DI:
	{
		handle->interrupt = -1;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c DI %*c", 5, ' ', 17, ' ');
	} break;

	case PUSH_AF:
	{
		PUSH(handle->F.val);
		PUSH(handle->A);

		handle->cycles = 16;
		handle->PC++;

		PRINT_DBG("%*c PUSH AF %*c", 5, ' ', 12, ' ');
	} break;

	case LD_ANN:
	{
		uint16_t addr = (*(handle->PC + 2) << 8) | (*(handle->PC + 1));
		handle->A = *(ram + addr);

		handle->cycles = 16;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD A, ($%04X) %*c", *(handle->PC - 2), *(handle->PC - 1), addr, 6, ' ');
	} break;

	case CP_IMM:
	{
		uint16_t tmp = (uint16_t)handle->A - *(handle->PC + 1);

		handle->F.zero = (tmp == 0);
		handle->F.negative = 1;
		handle->F.half_carry = ((tmp & 0x10) != (handle->A & 0x10));
		handle->F.carry = (tmp & 0x100);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%*c CP 0x%02X %*c", 5, ' ', *(handle->PC - 1), 12, ' ');
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


	PRINT_DBG("AF: %04X BC: %04X DE: %04X, HL: %04X SP: %04X I: %02X CYC: %llu\n", handle->AF, handle->BC, handle->DE, handle->HL, (uint16_t)(handle->SP - ram), *(ram + 0xFFFF), handle->total_cycles);

	return 1;
}