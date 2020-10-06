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

void readROM(const char* path, struct rom* buf, uint8_t* ram)
{
	FILE* fp = fopen(path, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "Failed to open file for reading: %s\n", path);
		return;
	}

	long size = 0;
	fseek(fp, 0L, SEEK_END);
	size = ftell(fp);
	rewind(fp);
	buf->data = ram;
	if (buf->data == NULL)
	{
		fprintf(stderr, "ROM memory allocation failed.\n");
		fclose(fp);
		return;
	}
	fread(buf->data, sizeof(uint8_t), size, fp);

	for (uint16_t i = 0; i < 0x100; i++)
		*(buf->data + i) = (uint8_t)BOOTLOADER[i];

	buf->entrypoint = buf->data + 0x0100;
	buf->nin_logo = buf->data + 0x0104;
	buf->title = buf->data + 0x0134;
	buf->man_code = buf->data + 0x013F;
	buf->cgb_flag = buf->data + 0x0143;
	buf->new_lic_code = buf->data + 0x0144;
	buf->sgb_flag = buf->data + 0x0146;
	buf->cartridge_type = buf->data + 0x0147;
	buf->rom_size = buf->data + 0x0148;
	buf->ram_size = buf->data + 0x0149;
	buf->destination = buf->data + 0x014A;
	buf->old_lic_code = buf->data + 0x014B;
	buf->mask_rom_ver_num = buf->data + 0x014C;
	buf->header_checksum = buf->data + 0x014D;
	buf->glob_checksum = buf->data + 0x014E;
	buf->code = buf->data + 0x0150;

	fclose(fp);
}