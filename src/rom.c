#include "rom.h"

void readROM(const char* path, struct rom* buf)
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
	buf->data = (uint8_t*)malloc(size);
	if (buf->data == NULL)
	{
		fprintf(stderr, "ROM memory allocation failed.\n");
		fclose(fp);
		return;
	}
	fread(buf->data, sizeof(uint8_t), size, fp);

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