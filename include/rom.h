#ifndef _ROM_H_
#define _ROM_H_

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

struct rom
{
	uint8_t* data;

	uint8_t* entrypoint;
	uint8_t* nin_logo;
	uint8_t* title;
	uint8_t* man_code;
	uint8_t* cgb_flag;
	uint8_t* new_lic_code;
	uint8_t* sgb_flag;
	uint8_t* cartridge_type;
	uint8_t* rom_size;
	uint8_t* ram_size;
	uint8_t* destination;
	uint8_t* old_lic_code;
	uint8_t* mask_rom_ver_num;
	uint8_t* header_checksum;
	uint8_t* glob_checksum;

	uint8_t* code;
};

void readROM(const char* path, struct rom* buf);

#endif // _ROM_H_