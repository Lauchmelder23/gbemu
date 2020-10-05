#include "cpu.h"
#include "gpu.h"
#include "rom.h"

uint8_t reset_cpu(struct cpu* handle, struct gpu* gpu, struct rom* rom, uint8_t* ram)
{
	handle->state = BOOT;
	handle->cycles = 1;
	handle->total_cycles = 0;
	handle->PC = rom->data;

	PRINT_DBG("----- STARTING BOOT SEQUENCE -----%*c\n", 0, ' ');
	while (handle->PC >= rom->data && handle->PC < rom->data + 256)
	{
		if (!exec_instr(handle, gpu, rom, ram)) return 0;
	}

	handle->PC = rom->data + 0x100;
	handle->state = RUN;
	PRINT_DBG("----- FINISHED BOOT SEQUENCE -----%*c\n", 0, ' ');
	return 1;
}

uint8_t handle_cb_opcode(struct cpu* handle, struct rom* rom, uint8_t* ram)
{
	uint8_t opcode = *(handle->PC + 1);
	PRINT_DBG("%02X %*c ", opcode, 2, ' ');

	uint8_t* reg = ram;
	char* arg = "";

	switch (opcode & 0x07)
	{
	case 0x0: reg = &(handle->B); handle->cycles = 8;		arg = "B";
	case 0x1: reg = &(handle->C); handle->cycles = 8;		arg = "C";
	case 0x2: reg = &(handle->D); handle->cycles = 8;		arg = "D";
	case 0x3: reg = &(handle->E); handle->cycles = 8;		arg = "E";
	case 0x4: reg = &(handle->H); handle->cycles = 8;		arg = "H";
	case 0x5: reg = &(handle->L); handle->cycles = 8;		arg = "L";
	case 0x6: reg = ram + handle->HL; handle->cycles = 16;	arg = "(HL)";
	case 0x7: reg = &(handle->A); handle->cycles = 8;		arg = "A";
	}

	switch (opcode & 0xF0)
	{
	case 0x10:
	{
		if ((opcode & 0x08) == 0)	// RL
		{
			handle->F.carry = ((*reg & 0x80) != 0);
			*reg <<= 1;
			handle->F.zero = (*reg == 0);
			PRINT_DBG("RL %s %*c", arg, 15, ' ');
		} else {	// RR
			handle->F.carry = ((*reg & 0x01) != 0);
			*reg >>= 1;
			handle->F.zero = (*reg == 0);
			PRINT_DBG("RR %s %*c", arg, 15, ' ');
		}
	} break;

	case 0x40:
	case 0x50:
	case 0x60:
	case 0x70:
	{
		uint8_t bitmask = 0x01 << (((((opcode & 0xF0) - 0x40) >> 4) * 2) + ((opcode & 0x08) >> 3));

		handle->F.zero = ((*reg & bitmask) == 0x00);
		handle->F.negative = 0;
		handle->F.half_carry = 1;
		
		PRINT_DBG("BIT %u, %s %*c", (((((opcode & 0xF0) - 0x40) >> 4) * 2) + ((opcode & 0x08) >> 3)), arg, 11, ' ');
	} break;

	default:
		fprintf(stderr, "Unknown opcode: %02X %02X\n", *(handle->PC), opcode);
		return 0;
	}

	handle->PC += 2;
	return 1;
}

uint8_t exec_instr(struct cpu* handle, struct gpu* gpu, struct rom* rom, uint8_t* ram)
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

	case INC_B:
	{
		uint8_t tmp = handle->B + 1;

		handle->F.zero = (handle->B == 0);
		handle->F.negative = 0;
		handle->F.half_carry = ((tmp & 0xF) | (handle->B & 0xF));

		handle->B = tmp;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c INC B %*c", 5, ' ', 14, ' ');
	} break;

	case DEC_B:
	{
		uint8_t tmp = handle->B - 1;

		handle->F.zero = (handle->B == 0);
		handle->F.negative = 1;
		handle->F.half_carry = ((tmp & 0x10) != (handle->B & 0x10));

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

	case INC_C:
	{
		uint8_t tmp = handle->C + 1;

		handle->F.zero = (handle->C == 0);
		handle->F.negative = 0;
		handle->F.half_carry = ((tmp & 0xF) | (handle->C & 0xF));

		handle->C = tmp;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c INC C %*c", 5, ' ', 14, ' ');
	} break;

	case DEC_C:
	{
		uint8_t tmp = handle->C - 1;

		handle->F.zero = (handle->C == 0);
		handle->F.negative = 1;
		handle->F.half_carry = ((tmp & 0x10) != (handle->C & 0x10));

		handle->C = tmp;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c DEC C %*c", 5, ' ', 14, ' ');
	} break;

	case LD_CN:
	{
		handle->C = *(handle->PC + 1);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LD C, 0x%02X %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 9, ' ');
	} break;

	case LD_DENN:
	{
		uint16_t val = ((uint16_t)(*(handle->PC + 2)) << 8) | *(handle->PC + 1);
		handle->DE = val;

		handle->cycles = 12;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD DE, 0x%04X %*c", *(handle->PC - 2), *(handle->PC - 1), val, 6, ' ');
	} break;

	case INC_DE:
	{
		handle->DE++;

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c INC DE %*c", 5, ' ', 13, ' ');
	} break;

	case RLA:
	{
		handle->F.carry = ((handle->A & 0x80) != 0);
		handle->A <<= 1;
		handle->F.zero = (handle->A == 0);

		handle->cycles = 4;
		handle->PC++;
		PRINT_DBG("%*c RLA %*c", 5, ' ', 16, ' ');
	} break;

	case JR_N:
	{
		int8_t offset = *(handle->PC + 1);

		PRINT_DBG("%02X %*c JR 0x%02X %*c", (uint8_t)offset, 2, ' ', (uint8_t)offset, 12, ' ');

		handle->cycles = 8;
		handle->PC += 2;
		handle->PC += offset;
	} break;

	case LD_ADE:
	{
		handle->A = *(ram + handle->DE);

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c LDI A, (DE) %*c", 5, ' ', 8, ' ');
	} break;

	case DEC_E:
	{
		uint8_t tmp = handle->E - 1;

		handle->F.zero = (handle->E == 0);
		handle->F.negative = 1;
		handle->F.half_carry = ((tmp & 0x10) != (handle->E & 0x10));

		handle->E = tmp;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c DEC E %*c", 5, ' ', 14, ' ');
	} break;

	case LD_EN:
	{
		handle->E = *(handle->PC + 1);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LD E, 0x%02X %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 9, ' ');
	} break;

	case LD_HLNN:
	{
		uint16_t val = ((uint16_t)(*(handle->PC + 2)) << 8) | *(handle->PC + 1);
		handle->HL = val;

		handle->cycles = 12;
		handle->PC += 3;

		PRINT_DBG("%02X %02X LD HL, 0x%04X %*c", *(handle->PC - 2), *(handle->PC - 1), val, 6, ' ');
	} break;

	case LDI_HLA:
	{
		*(ram + handle->HL++) = handle->A;

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c LDI (HL), A %*c", 5, ' ', 8, ' ');
	} break;

	case INC_HL:
	{
		handle->HL++;

		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c INC HL %*c", 5, ' ', 13, ' ');
	} break;

	case INC_H:
	{
		uint8_t tmp = handle->H + 1;

		handle->F.zero = (handle->H == 0);
		handle->F.negative = 1;
		handle->F.half_carry = ((tmp & 0x10) != (handle->H & 0x10));

		handle->H = tmp;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c INC A %*c", 5, ' ', 14, ' ');
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

	case LD_LN:
	{
		handle->L = *(handle->PC + 1);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LD L, 0x%02X %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 9, ' ');
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

	case DEC_A:
	{
		uint8_t tmp = handle->A - 1;

		handle->F.zero = (handle->A == 0);
		handle->F.negative = 1;
		handle->F.half_carry = ((tmp & 0x10) != (handle->A & 0x10));

		handle->A = tmp;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c DEC A %*c", 5, ' ', 14, ' ');
	} break;

	case LD_AI:
	{
		handle->A = *(handle->PC + 1);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%02X %*c LD A, 0x%02X %*c", *(handle->PC - 1), 2, ' ', *(handle->PC - 1), 9, ' ');
	} break;

	case LD_CA:
	{
		handle->C = handle->A;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD C, A %*c", 5, ' ', 12, ' ');
	} break;

	case LD_DA:
	{
		handle->D = handle->A;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD D, A %*c", 5, ' ', 12, ' ');
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

	case LD_HA:
	{
		handle->H = handle->A;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD H, A %*c", 5, ' ', 12, ' ');
	} break;

	case LD_HLA:
	{
		*(ram + handle->HL) = handle->A;
		
		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c LD (HL), A %*c", 5, ' ', 9, ' ');
	} break;

	case LD_AB:
	{
		handle->A = handle->B;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD A, B %*c", 5, ' ', 12, ' ');

	} break;

	case LD_AE:
	{
		handle->A = handle->E;

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c LD A, E %*c", 5, ' ', 12, ' ');
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

	case XOR_A:
	{
		handle->A ^= handle->A;

		handle->F.val = 0x00;
		handle->F.zero = (handle->A == 0x00);

		handle->cycles = 4;
		handle->PC++;

		PRINT_DBG("%*c XOR A, A %*c", 5, ' ', 11, ' ');
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

	case SHOOT_ME:	// Please do it
	{
		if (!handle_cb_opcode(handle, rom, ram))
			return 0;
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

	case LD_RCA:
	{
		*(ram + 0xFF00 + handle->C) = handle->A;
		
		handle->cycles = 8;
		handle->PC++;

		PRINT_DBG("%*c LD ($FF00+%02X), A %*c", 5, ' ', handle->C, 3, ' ');
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
		handle->F.carry = ((tmp & 0x100) != 0);

		handle->cycles = 8;
		handle->PC += 2;

		PRINT_DBG("%*c CP 0x%02X %*c", 5, ' ', *(handle->PC - 1), 12, ' ');
	} break;

	default:
		fprintf(stderr, "Unknown opcode: %02X", opcode);
		return 0;
	}

	if (interruptSet)
	{
		*(ram + 0xFFFF) = (interruptSet == 1) ? (uint8_t)1 : (uint8_t)0;
		handle->interrupt = 0;
	}

	tick_gpu(gpu, handle->cycles);

	PRINT_DBG("AF: %04X BC: %04X DE: %04X HL: %04X SP: %04X I: %02X Y: %02X CYC: %llu\n", handle->AF, handle->BC, handle->DE, handle->HL, (uint16_t)(handle->SP - ram), *(ram + 0xFFFF), *(gpu->curline), handle->total_cycles);

	return 1;
}