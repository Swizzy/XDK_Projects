#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <xtl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "xenon_sfcx.h"
#include "Corona4G.h"

#include "OutputConsole.h"

struct sfc sfc = {0};
unsigned char* blockbuf;

FILE * fd;

static unsigned long bswap32(unsigned long t) { return ((t & 0xFF) << 24) | ((t & 0xFF00) << 8) | ((t & 0xFF0000) >> 8) | ((t & 0xFF000000) >> 24); }

__declspec( naked ) void sfcx_writereg(int addr, unsigned long data)
 {
    __asm
    {
        li          r6, 0x00
        lis         r5, 0x7FEA          //%r5=0x0000_0000_7FEA_0000
        ori         r5, r5, 0xC000      //%r5=0x0000_0000_7FEA_C000
        or          r5, r5, r3          //%r5=0x0000_0000_7FEA_C00X Add the register offset
        stwbrx      r4, r6, r5          //store %r4 le to address pointed to by %r5
        //eieio                         //Enforce In-Order Execution of I/O
        blr
    }
 }

__declspec( naked ) unsigned long sfcx_readreg(int addr)
 {
    __asm
    {
        li          r5, 0x00
        lis         r4, 0x7FEA          //%r4=0x0000_0000_7FEA_0000
        ori         r4, r4, 0xC000      //%r4=0x0000_0000_7FEA_C000
        or          r4, r4, r3          //%r4=0x0000_0000_7FEA_C00X Add the register offset
        lwbrx       r3, r5, r4          //load data from address pointed to by %r4 into %r3 le
        //eieio                         //Enforce In-Order Execution of I/O
        blr
    }
 }

int sfcx_read_page(unsigned char *data, int address, int raw)
{
	int status;
	int i;
	int page_sz;

	sfcx_writereg(SFCX_STATUS, sfcx_readreg(SFCX_STATUS));

	// Set flash address (logical)
	//address &= 0x3fffe00; // Align to page
	sfcx_writereg(SFCX_ADDRESS, address);

	// Command the read
	// Either a logical read (0x200 bytes, no meta data)
	// or a Physical read (0x210 bytes with meta data)
	sfcx_writereg(SFCX_COMMAND, raw ? PHY_PAGE_TO_BUF : LOG_PAGE_TO_BUF);

	// Wait Busy
	while ((status = sfcx_readreg(SFCX_STATUS)) & STATUS_BUSY);

	if (!SFCX_SUCCESS(status))
	{
		if (status & STATUS_BB_ER)
			dprintf(" ! SFCX: Bad block found at %08X\n", sfcx_address_to_block(address));
		else if (status & STATUS_ECC_ER)
			//	dprintf(" ! SFCX: (Corrected) ECC error at address %08X: %08X\n", address, status);
			status = status;
		else if (!raw && (status & STATUS_ILL_LOG))
			dprintf(" ! SFCX: Illegal logical block at %08X (status: %08X)\n", sfcx_address_to_block(address), status);
		else
			dprintf(" ! SFCX: Unknown error at address %08X: %08X. Please worry.\n", address, status);
	}

	// Set internal page buffer pointer to 0
	sfcx_writereg(SFCX_ADDRESS, 0);

	page_sz = raw ? sfc.page_sz_phys : sfc.page_sz;

	for (i = 0; i < page_sz ; i += 4)
	{
		// Transfer data from buffer to register
		sfcx_writereg(SFCX_COMMAND, PAGE_BUF_TO_REG);

		// Read out our data through the register
		*(int*)(data + i) = bswap32(sfcx_readreg(SFCX_DATA));
	}

	return status;
}

int sfcx_write_page(unsigned char *data, int address)
{
	int i;
	int status;

	sfcx_writereg(SFCX_STATUS, 0xFF);

	// Enable Writes
	sfcx_writereg(SFCX_CONFIG, sfcx_readreg(SFCX_CONFIG) | CONFIG_WP_EN);

	// Set internal page buffer pointer to 0
	sfcx_writereg(SFCX_ADDRESS, 0);


	for (i = 0; i < sfc.page_sz_phys; i+=4)
	{
		// Write out our data through the register
		sfcx_writereg(SFCX_DATA, bswap32(*(int*)(data + i)));

		// Transfer data from register to buffer
		sfcx_writereg(SFCX_COMMAND, REG_TO_PAGE_BUF);
	}

	// Set flash address (logical)
	//address &= 0x3fffe00; // Align to page
	sfcx_writereg(SFCX_ADDRESS, address);

	// Unlock sequence (for write)
	sfcx_writereg(SFCX_COMMAND, UNLOCK_CMD_0);
	sfcx_writereg(SFCX_COMMAND, UNLOCK_CMD_1);

	// Wait Busy
	while (sfcx_readreg(SFCX_STATUS) & STATUS_BUSY);

	// Command the write
	sfcx_writereg(SFCX_COMMAND, WRITE_PAGE_TO_PHY);

	// Wait Busy
	while (sfcx_readreg(SFCX_STATUS) & STATUS_BUSY);

	status = sfcx_readreg(SFCX_STATUS);
	if (!SFCX_SUCCESS(status))
		dprintf(" ! SFCX: Unexpected sfcx_write_page status %08X\n", status);

	// Disable Writes
	sfcx_writereg(SFCX_CONFIG, sfcx_readreg(SFCX_CONFIG) & ~CONFIG_WP_EN);

	return status;
}

int sfcx_read_block(unsigned char *data, int address, int raw)
{
	int p;
	int status = 0;
	int page_sz = raw ? sfc.page_sz_phys : sfc.page_sz;

	for (p = 0; p < sfc.pages_in_block; p++)
	{
		status |= sfcx_read_page(&data[p * page_sz], address + (p * sfc.page_sz), raw);
		//if (!SFCX_SUCCESS(status))
		//	break;
	}
	return status;
}

int sfcx_write_block(unsigned char *data, int address)
{
	int p;
	int status = 0;

	for (p = 0; p < sfc.pages_in_block; p++)
	{
		status |= sfcx_write_page(&data[p * sfc.page_sz_phys], address + (p * sfc.page_sz));
		//if (!SFCX_SUCCESS(status))
		//	break;
	}
	return status;
}

int sfcx_erase_block(int address)
{
	int status;

	// Enable Writes
	sfcx_writereg(SFCX_CONFIG, sfcx_readreg(SFCX_CONFIG) | CONFIG_WP_EN);
	sfcx_writereg(SFCX_STATUS, 0xFF);

	// Set flash address (logical)
	//address &= 0x3fffe00; // Align to page
	sfcx_writereg(SFCX_ADDRESS, address);

	// Wait Busy
	while (sfcx_readreg(SFCX_STATUS) & STATUS_BUSY);

	// Unlock sequence (for erase)
	sfcx_writereg(SFCX_COMMAND, UNLOCK_CMD_1);
	sfcx_writereg(SFCX_COMMAND, UNLOCK_CMD_0);

	// Wait Busy
	while (sfcx_readreg(SFCX_STATUS) & STATUS_BUSY);

	// Command the block erase
	sfcx_writereg(SFCX_COMMAND, BLOCK_ERASE);

	// Wait Busy
	while (sfcx_readreg(SFCX_STATUS) & STATUS_BUSY);

	status = sfcx_readreg(SFCX_STATUS);
	//if (!SFCX_SUCCESS(status))
	//	dprintf(" ! SFCX: Unexpected sfcx_erase_block status %08X\n", status);
	sfcx_writereg(SFCX_STATUS, 0xFF);

	// Disable Writes
	sfcx_writereg(SFCX_CONFIG, sfcx_readreg(SFCX_CONFIG) & ~CONFIG_WP_EN);

	return status;
}

void sfcx_calcecc(unsigned int *data)
{
	unsigned int i=0, val=0;
	unsigned char *edc = ((unsigned char*)data) + sfc.page_sz;

	unsigned int v=0;

	for (i = 0; i < 0x1066; i++)
	{
		if (!(i & 31))
			v = ~bswap32(*data++);
		val ^= v & 1;
		v>>=1;
		if (val & 1)
			val ^= 0x6954559;
		val >>= 1;
	}

	val = ~val;

	// 26 bit ecc data
	edc[0xC] = ((val << 6) | (edc[0xC] & 0x3F)) & 0xFF;
	edc[0xD] = (val >> 2) & 0xFF;
	edc[0xE] = (val >> 10) & 0xFF;
	edc[0xF] = (val >> 18) & 0xFF;
}

int sfcx_get_blocknumber(unsigned char *data)
{
	int num = 0;
	switch (sfc.meta_type)
	{
	case META_TYPE_0:
		num = (data[sfc.page_sz + 0x1] << 8) | (data[sfc.page_sz + 0x0]);
		break;
	case META_TYPE_1:
		num = (data[sfc.page_sz + 0x2] << 8) | (data[sfc.page_sz + 0x1]);
		break;
	case META_TYPE_2:
		num = (data[sfc.page_sz + 0x2] << 8) | (data[sfc.page_sz + 0x1]);
		break;
	}
	return num;
}

void sfcx_set_blocknumber(unsigned char *data, int num)
{
	switch (sfc.meta_type)
	{
	case META_TYPE_0:
		data[sfc.page_sz + 0x1] = (num >> 8) & 0xFF;
		data[sfc.page_sz + 0x0] = (num >> 0) & 0xFF;
		break;
	case META_TYPE_1:
		data[sfc.page_sz + 0x2] = (num >> 8) & 0xFF;
		data[sfc.page_sz + 0x1] = (num >> 0) & 0xFF;
		break;
	case META_TYPE_2:
		data[sfc.page_sz + 0x2] = (num >> 8) & 0xFF;
		data[sfc.page_sz + 0x1] = (num >> 0) & 0xFF;
		break;
	}
}

int sfcx_get_blockversion(unsigned char *data)
{
	int ver = 0;
	switch (sfc.meta_type)
	{
	case META_TYPE_0:
		ver = (data[sfc.page_sz + 0x6] << 24) | (data[sfc.page_sz + 0x4] << 16) |
			(data[sfc.page_sz + 0x3] << 8)  | (data[sfc.page_sz + 0x2]);
		break;
	case META_TYPE_1:
		ver = (data[sfc.page_sz + 0x6] << 24) | (data[sfc.page_sz + 0x4] << 16) |
			(data[sfc.page_sz + 0x3] << 8)  | (data[sfc.page_sz + 0x0]);
		break;
	case META_TYPE_2:
		ver = (data[sfc.page_sz + 0x6] << 24) | (data[sfc.page_sz + 0x4] << 16) |
			(data[sfc.page_sz + 0x3] << 8)  | (data[sfc.page_sz + 0x5]);
		break;
	}
	return ver;
}

void sfcx_set_blockversion(unsigned char *data, int ver)
{
	switch (sfc.meta_type)
	{
	case META_TYPE_0:
		data[sfc.page_sz + 0x2] = (ver >> 0)  & 0xFF;
		data[sfc.page_sz + 0x3] = (ver >> 8)  & 0xFF;
		data[sfc.page_sz + 0x4] = (ver >> 16) & 0xFF;
		data[sfc.page_sz + 0x6] = (ver >> 24) & 0xFF;
		break;
	case META_TYPE_1:
		data[sfc.page_sz + 0x0] = (ver >> 0)  & 0xFF;
		data[sfc.page_sz + 0x3] = (ver >> 8)  & 0xFF;
		data[sfc.page_sz + 0x4] = (ver >> 16) & 0xFF;
		data[sfc.page_sz + 0x6] = (ver >> 24) & 0xFF;
		break;
	case META_TYPE_2:
		data[sfc.page_sz + 0x5] = (ver >> 0)  & 0xFF;
		data[sfc.page_sz + 0x3] = (ver >> 8)  & 0xFF;
		data[sfc.page_sz + 0x4] = (ver >> 16) & 0xFF;
		data[sfc.page_sz + 0x6] = (ver >> 24) & 0xFF;
		break;
	}
}

void sfcx_set_pagevalid(unsigned char *data)
{
	switch (sfc.meta_type)
	{
	case META_TYPE_0:
		data[sfc.page_sz + 0x5] = 0xFF;
		break;
	case META_TYPE_1:
		data[sfc.page_sz + 0x5] = 0xFF;
		break;
	case META_TYPE_2:
		data[sfc.page_sz + 0x0] = 0xFF;
		break;
	}
}

void sfcx_set_pageinvalid(unsigned char *data)
{
	switch (sfc.meta_type)
	{
	case META_TYPE_0:
		data[sfc.page_sz + 0x5] = 0x00;
		break;
	case META_TYPE_1:
		data[sfc.page_sz + 0x5] = 0x00;
		break;
	case META_TYPE_2:
		data[sfc.page_sz + 0x0] = 0x00;
		break;
	}
}

int sfcx_is_pagevalid(unsigned char *data)
{
	int valid = 0;
	switch (sfc.meta_type)
	{
	case META_TYPE_0:
		valid = data[sfc.page_sz + 0x5] == 0xFF;
		break;
	case META_TYPE_1:
		valid = data[sfc.page_sz + 0x5] == 0xFF;
		break;
	case META_TYPE_2:
		valid = data[sfc.page_sz + 0x0] == 0xFF;
		break;
	}
	return valid;
}

int sfcx_is_pagezeroed(unsigned char *data)
{
	int i;
	for(i = 0; i < sfc.page_sz; i++)
	{
		if (data[i]!=0x00)
			return 0;
	}
	return 1;
}

int sfcx_is_pageerased(unsigned char *data)
{
	int i;
	for(i = 0; i < sfc.page_sz_phys; i++)
	{
		if (data[i]!=0xFF)
			return 0;
	}
	return 1;
}

int sfcx_block_to_address(int block) { return block * sfc.block_sz; }

int sfcx_address_to_block(int address) { return address / sfc.block_sz; }

int sfcx_block_to_rawaddress(int block) { return block * sfc.block_sz_phys; }

int sfcx_rawaddress_to_block(int address) { return address / sfc.block_sz_phys; }

int rawflash_writeImage(int len, FILE * fd)
{
	int i=0;
	int secondPgOffset = sfc.page_sz_phys;
	int addr, addrphy, status, r;
	int readsz = sfc.pages_in_block*sfc.page_sz_phys;
	int numblocks = (len/sfc.block_sz_phys);
	blockbuf = (unsigned char*)malloc(readsz);
	if(blockbuf == NULL)
	{
		dprintf("ERROR: unable to allocate 0x%x bytes for a buffer!\n", readsz);
		return 0;
	}
	if(sfc.meta_type == META_TYPE_2)
		secondPgOffset = 0x1080; // 0x210*8
	while(i < numblocks)
	{
		KillControllers();
		dprintf("\rprocessing block 0x%04x of 0x%04x (%3.2f %%)", i+1, numblocks, ((float)(i + 1) / (float)numblocks) * 100);
		addr = i*sfc.block_sz;
		// check first two pages of each block to find out if it's a good block
		status = sfcx_read_block(blockbuf, addr, 1);
		if((sfcx_is_pagevalid(blockbuf) == 0) || (sfcx_is_pagevalid(&blockbuf[secondPgOffset]) == 0))
			status = status | STATUS_BB_ER;
		r = fread( blockbuf, readsz, 1, fd);
		if (r < 0)
		{
			dprintf("\rERROR: failed to read %d bytes from file\n\n",readsz);
			return 0;
		}
		if((status & (STATUS_BB_ER|STATUS_ECC_ER)) == 0)
		{
			addr = i*sfc.block_sz_phys;
			addrphy = i*sfc.block_sz;
			sfcx_erase_block(addrphy);
			sfcx_write_block(blockbuf, addrphy);
		}
		else
			dprintf("\rblock 0x%x seems bad, status 0x%08x\n", i, status);
		i++;
	}
	dprintf("\r\n\n");
	return 1;
}

int rawflash_readImage(int len, FILE * fd)
{
	int i=0;
	int secondPgOffset = sfc.page_sz_phys;
	int addr, status;
	int readsz = sfc.pages_in_block*sfc.page_sz_phys;
	int numblocks = (len/sfc.block_sz_phys);
	blockbuf = malloc(readsz);
	if(blockbuf == NULL)
	{
		dprintf("ERROR: unable to allocate 0x%x bytes for a buffer!\n", readsz);
		return 0;
	}
	if(sfc.meta_type == META_TYPE_2)
		secondPgOffset = 0x1080; // 0x210*8
	while(i < numblocks)
	{
		dprintf("\rprocessing block 0x%04x of 0x%04x (%3.2f %%)", i+1, numblocks, ((float)(i + 1) / (float)numblocks) * 100);
		addr = i*sfc.block_sz;
		// check first two pages of each block to find out if it's a good block
		status = sfcx_read_block(blockbuf, addr, 1);
		if((sfcx_is_pagevalid(blockbuf) == 0) || (sfcx_is_pagevalid(&blockbuf[secondPgOffset]) == 0))
			status = status | STATUS_BB_ER;
		if (fwrite( blockbuf, readsz, 1, fd) < 0)
		{
			dprintf("\rERROR: failed to write %d bytes to file\n\n",readsz);
			return 0;
		}
		if((status & (STATUS_BB_ER|STATUS_ECC_ER)) != 0) dprintf("\rblock 0x%x seems bad, status 0x%08x\n", i, status);
		i++;
	}
	dprintf("\r\n\n");
	return 1;
}

void try_rawflash(char *filename)
{
	FILE * fd;
	struct __stat64 s;
	int size;
	if (fopen_s(&fd, filename, "rb") != 0)
	{
		dprintf("File %s not found\n", filename);
		return;
	}
	dprintf(" * rawflash v4 started (by cOz)\n");

	_stat64(filename, &s);
	size = (int)(s.st_size&0xFFFFFFFF);
	if((size == (RAW_NAND_64*4)) || (size == (RAW_NAND_64*8))) // 256 or 512M NAND image, only flash 64M	
		size = RAW_NAND_64;
	else if((size != 0x1080000)&& (size != RAW_NAND_64)) // 16 M size
	{
		dprintf("error: %s - size %d is not valid image size!\n", filename, size);
		fclose(fd);
		return;
	}
	dprintf("%s opened OK, attempting to write 0x%x bytes to flash...\n",filename, size);
	if(rawflash_writeImage(size, fd) == 1)
		dprintf("image written :D\n");
	else
		dprintf("failed to write image :(\n");

	fclose(fd);
	if(blockbuf != NULL)
		free(blockbuf);
}

void try_rawdump(char *filename, int size)
{
	FILE * fd;
	if (fopen_s(&fd, filename, "wb") != 0)
	{
		dprintf("ERROR: Unable to open %s for write\n", filename);
		return;
	}
	if (size == 0)
	{
		size = sfc.size_bytes_phys;
		if((size == (RAW_NAND_64*4)) || (size == (RAW_NAND_64*8))) // 256 or 512M NAND image, only flash 64M
			size = RAW_NAND_64;
		else if((size != 0x1080000)&& (size != RAW_NAND_64)) // 16 M size
		{
			dprintf("error: size %d is not supported!\n", size);
			fclose(fd);
			return;
		}
	}
	dprintf(" * rawdump v1 started (by Swizzy)\n");
	dprintf("%s opened OK, attempting to read 0x%x bytes from flash...\n",filename, size);
	if(rawflash_readImage(size, fd) == 1)
		dprintf("NAND Dumped! :D\n");
	else
		dprintf("failed to dump NAND :(\n");

	fclose(fd);
	if(blockbuf != NULL)
		free(blockbuf);
}

unsigned int sfcx_init(void)
{
	unsigned int config = sfcx_readreg(SFCX_CONFIG);

	if (sfc.initialized) return config;

	sfc.initialized = 0;
	sfc.meta_type = 0;
	sfc.page_sz = 0x200;
	sfc.meta_sz = 0x10;
	sfc.page_sz_phys = sfc.page_sz + sfc.meta_sz;

	//Turn off interrupts, turn off WP_EN, and set DMA pages to 0
	sfcx_writereg(SFCX_CONFIG, config &~ (CONFIG_INT_EN|CONFIG_WP_EN|CONFIG_DMA_LEN));

	switch ((config >> 17) & 0x03)
	{
	case 0: // Small block original SFC (pre jasper)
		sfc.meta_type = META_TYPE_0;
		sfc.blocks_per_lg_block = 8;

		switch ((config >> 4) & 0x3)
		{
		case 0: // Unsupported 8MB?
			dprintf(" ! SFCX: Unsupported Type A-0\n");
			Sleep(5);
			return 1;

			//sfc.block_sz = 0x4000; // 16 KB
			//sfc.size_blocks = 0x200;
			//sfc.size_bytes = sfc.size_blocks << 0xE;
			//sfc.size_usable_fs = 0xXXX;
			//sfc.addr_config = 0x07BE000 - 0x4000;

		case 1: // 16MB
			sfc.block_sz = 0x4000; // 16 KB
			sfc.size_blocks = 0x400;
			sfc.size_bytes = sfc.size_blocks << 0xE;
			sfc.size_usable_fs = 0x3E0;
			sfc.addr_config = (sfc.size_usable_fs - CONFIG_BLOCKS) * sfc.block_sz;
			break;

		case 2: // 32MB
			sfc.block_sz = 0x4000; // 16 KB
			sfc.size_blocks = 0x800;
			sfc.size_bytes = sfc.size_blocks << 0xE;
			sfc.size_usable_fs = 0x7C0;
			sfc.addr_config = (sfc.size_usable_fs - CONFIG_BLOCKS) * sfc.block_sz;
			break;

		case 3: // 64MB
			sfc.block_sz = 0x4000; // 16 KB
			sfc.size_blocks = 0x1000;
			sfc.size_bytes = sfc.size_blocks << 0xE;
			sfc.size_usable_fs = 0xF80;
			sfc.addr_config = (sfc.size_usable_fs - CONFIG_BLOCKS) * sfc.block_sz;
			break;
		}
		break;

	case 1: // New SFC/Southbridge: Codename "Panda"?
	case 2: // New SFC/Southbridge: Codename "Panda" v2?
		switch ((config >> 4) & 0x3)
		{
		case 0: 

			if(((config >> 17) & 0x03) == 0x01)
			{
				// Unsupported
				sfc.meta_type = META_TYPE_0;
				dprintf(" ! SFCX: Unsupported Type B-0\n");
				Sleep(5);
				return 2;
			}
			else
			{
				sfc.meta_type = META_TYPE_1;
				sfc.block_sz = 0x4000; // 16 KB
				sfc.size_blocks = 0x400;
				sfc.size_bytes = sfc.size_blocks << 0xE;
				sfc.blocks_per_lg_block = 8;
				sfc.size_usable_fs = 0x3E0;
				sfc.addr_config = (sfc.size_usable_fs - CONFIG_BLOCKS) * sfc.block_sz;
				break;
			}

		case 1: 

			if(((config >> 17) & 0x03) == 0x01)
			{
				// Small block 16MB setup
				sfc.meta_type = META_TYPE_1;
				sfc.block_sz = 0x4000; // 16 KB
				sfc.size_blocks = 0x400;
				sfc.size_bytes = sfc.size_blocks << 0xE;
				sfc.blocks_per_lg_block = 8;
				sfc.size_usable_fs = 0x3E0;
				sfc.addr_config = (sfc.size_usable_fs - CONFIG_BLOCKS) * sfc.block_sz;
				break;
			}
			else
			{
				// Small block 64MB setup
				sfc.meta_type = META_TYPE_1;
				sfc.block_sz = 0x4000; // 16 KB
				sfc.size_blocks = 0x1000;
				sfc.size_bytes = sfc.size_blocks << 0xE;
				sfc.blocks_per_lg_block = 8;
				sfc.size_usable_fs = 0xF80;
				sfc.addr_config = (sfc.size_usable_fs - CONFIG_BLOCKS) * sfc.block_sz;
				break;
			}

		case 2: // Large Block: Current Jasper 256MB and 512MB
			sfc.meta_type = META_TYPE_2;
			sfc.block_sz = 0x20000; // 128KB
			sfc.size_bytes = 0x1 << (((config >> 19) & 0x3) + ((config >> 21) & 0xF) + 0x17);
			sfc.size_blocks = sfc.size_bytes >> 0x11;
			sfc.blocks_per_lg_block = 1;
			sfc.size_usable_fs = 0x1E0;
			sfc.addr_config = (sfc.size_usable_fs - CONFIG_BLOCKS) * sfc.block_sz;
			break;

		case 3: // Large Block: Future or unknown hardware
			sfc.meta_type = META_TYPE_2;
			sfc.block_sz = 0x40000; // 256KB
			sfc.size_bytes = 0x1 << (((config >> 19) & 0x3) + ((config >> 21) & 0xF) + 0x17);
			sfc.size_blocks = sfc.size_bytes >> 0x12;
			sfc.blocks_per_lg_block = 1;
			sfc.size_usable_fs = 0xF0;
			sfc.addr_config = (sfc.size_usable_fs - CONFIG_BLOCKS) * sfc.block_sz;
			break;
		}
		break;

	default:
		dprintf(" ! SFCX: Unsupported Type\n");
		Sleep(5);
		return 3;
	}

	sfc.len_config = sfc.block_sz * 0x04; //4 physical blocks

	sfc.pages_in_block = sfc.block_sz / sfc.page_sz;
	sfc.block_sz_phys = sfc.pages_in_block * sfc.page_sz_phys;

	sfc.size_pages = sfc.size_bytes / sfc.page_sz;
	sfc.size_blocks = sfc.size_bytes / sfc.block_sz;

	sfc.size_bytes_phys = sfc.block_sz_phys * sfc.size_blocks;
	sfc.size_mb = sfc.size_bytes >> 20;

	/*
	int meta_type;

	meta_type = sfcx_read_metadata_type();
	if (meta_type == -1){
	dprintf(" ! SFCX: Meta Type detection error\n");
	delay(5);
	return 4;
	}

	if (meta_type != sfc.meta_type){
	dprintf(" ! SFCX: Meta Type detection difference\n");
	dprintf(" ! SFCX: expecting type: '%d' detected: '%d'\n", sfc.meta_type, meta_type);
	sfc.meta_type = meta_type;
	}

	*/
	sfc.initialized = SFCX_INITIALIZED;
	return config;
}

unsigned int sfcx_getconf(void) { return sfcx_readreg(SFCX_CONFIG); }

void sfcx_setconf(unsigned int config) { sfcx_writereg(SFCX_CONFIG, config); }

void sfcx_printinfo(unsigned int config)
{
	dprintf("   config register     = %08X\n", config);

	dprintf("   sfc:page_sz         = %08X\n", sfc.page_sz);
	dprintf("   sfc:meta_sz         = %08X\n", sfc.meta_sz);
	dprintf("   sfc:page_sz_phys    = %08X\n", sfc.page_sz_phys);

	dprintf("   sfc:pages_in_block  = %08X\n", sfc.pages_in_block);
	dprintf("   sfc:block_sz        = %08X\n", sfc.block_sz);
	dprintf("   sfc:block_sz_phys   = %08X\n", sfc.block_sz_phys);

	dprintf("   sfc:size_mb         = %dMB\n", sfc.size_mb);
	dprintf("   sfc:size_bytes      = %08X\n", sfc.size_bytes);
	dprintf("   sfc:size_bytes_phys = %08X\n", sfc.size_bytes_phys);

	dprintf("   sfc:size_pages      = %08X\n", sfc.size_pages);
	dprintf("   sfc:size_blocks     = %08X\n", sfc.size_blocks);
	dprintf("\n");
}

int sfcx_detecttype(void)
{
	unsigned int config = sfcx_readreg(0x7FEAC0FC);
	if (config == 0)
	{
		dprintf(" * Detected RAW NAND device!\n * Entering NAND Mode...\n\n");
		return 0;
	}
	dprintf(" * Detected MMC NAND device!\n * Entering MMC/Corona v2 [4GB] Mode...\n\n");
	return 1;
}