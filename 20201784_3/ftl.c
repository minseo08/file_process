#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "blockmapping.h"

extern void dd_erase(int pbn);
extern void dd_read(int ppn, char *pagebuf);
extern void dd_write(int ppn, char *pagebuf);

int mapping_table[DATABLKS_PER_DEVICE];
int free_block_pbn = 0;
int assign_block = 0;

void ftl_open()
{
	for (int i = 0; i < DATABLKS_PER_DEVICE; i++){
		mapping_table[i] = -1;
	}
	free_block_pbn = 0;
	return;
}

void ftl_read(int lsn, char *sectorbuf)
{
	int lbn = lsn / PAGES_PER_BLOCK;
	int offset = lsn % PAGES_PER_BLOCK;
	int pbn = mapping_table[lbn + 1];
	char pagebuf[PAGE_SIZE];
	dd_read(pbn * PAGES_PER_BLOCK + offset, pagebuf);
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
	return;
}

void ftl_write(int lsn, char *sectorbuf)
{
	int lbn = lsn / PAGES_PER_BLOCK;
	int offset = lsn % PAGES_PER_BLOCK;
	int pbn = mapping_table[lbn + 1];
	if (pbn == -1){
		assign_block++;
		mapping_table[lbn + 1] = assign_block;
		pbn = assign_block;
		char pagebuf[PAGE_SIZE];
		memset(pagebuf, 0xFF, PAGE_SIZE);
		memcpy(pagebuf, sectorbuf, strlen(sectorbuf));
		memcpy(pagebuf + SECTOR_SIZE, &lsn, sizeof(int));
		dd_write(pbn * PAGES_PER_BLOCK + offset, pagebuf);
	}
	else {
		char pagebuf[PAGE_SIZE];
		char sparebuf[SPARE_SIZE];
		dd_read(pbn * PAGES_PER_BLOCK + offset, pagebuf);
		char spare_byte = pagebuf[SECTOR_SIZE];
		if (spare_byte == (char)0xFF){
			char pagebuf[PAGE_SIZE];
			memset(pagebuf, 0xFF, PAGE_SIZE);
			memcpy(pagebuf, sectorbuf, strlen(sectorbuf));
			memcpy(pagebuf + SECTOR_SIZE, &lsn, sizeof(int));
			dd_write(pbn * PAGES_PER_BLOCK + offset, pagebuf);
		}
		else {
			int new_pbn = free_block_pbn;
			free_block_pbn = pbn;
			mapping_table[lbn + 1] = new_pbn;
			dd_erase(new_pbn);
			for (int i = 0; i < PAGES_PER_BLOCK; i++){
				if (i != offset){
					char copybuf[PAGE_SIZE];
					dd_read(pbn * PAGES_PER_BLOCK + i, copybuf);
					char spare_byte = copybuf[SECTOR_SIZE];
					if (spare_byte != (char)0xFF){
						dd_write(new_pbn * PAGES_PER_BLOCK + i, copybuf);
					}
				}
			}
			dd_erase(pbn);
			char pagebuf[PAGE_SIZE];
			memset(pagebuf, 0xFF, PAGE_SIZE);
			memcpy(pagebuf, sectorbuf, strlen(sectorbuf));
			memcpy(pagebuf + SECTOR_SIZE, &lsn, sizeof(int));
			dd_write(new_pbn * PAGES_PER_BLOCK + offset, pagebuf);
		}
	}
	return;
}

void ftl_print()
{
	printf("lbn pbn \n");
	for (int i = 0 ; i < DATABLKS_PER_DEVICE; i++){
		printf("%d %d\n", i, mapping_table[i]);
	}
	printf("free block=%d\n", free_block_pbn);
	return;
}
