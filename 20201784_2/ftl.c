#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "flash.h"

FILE *flashfp; // fdevicedriver.c에서 사용

extern int dd_read(int ppn, char *pagebuf);
extern int dd_write(int ppn, char *pagebuf);
extern int dd_erase(int pbn);

void fm_create(const char *filename, int num_blocks, char *blockbuf){
	flashfp = fopen(filename, "w");
	if (!flashfp){
		fprintf(stderr, "creating flash memory file error\n");
		exit(EXIT_FAILURE);
	}
	if (blockbuf != NULL){
		memset((void*)blockbuf, (char)0xFF, BLOCK_SIZE);
	}
	for (int i = 0; i < num_blocks; i++){
		fwrite((void*)blockbuf, BLOCK_SIZE, 1, flashfp);	
	}
	fclose(flashfp);
}

void page_write(const char *filename, char *pagebuf, int ppn, const char *sectordata, const char *sparedata, int *writes){
	flashfp = fopen(filename, "rb+");
	if (!flashfp) {
		fprintf(stderr, "opening flash memory file error\n");
		exit(EXIT_FAILURE);
	}
	size_t sector_len = strlen(sectordata);
	size_t spare_len = strlen(sparedata);
	if (sector_len > SECTOR_SIZE)
		sector_len = SECTOR_SIZE;
	memcpy(pagebuf, sectordata, sector_len);
	if (spare_len > SPARE_SIZE)
		spare_len = SPARE_SIZE;
	memcpy(pagebuf + SECTOR_SIZE, sparedata, spare_len);
	dd_write(ppn, pagebuf);
	(*writes)++;
	fclose(flashfp);
}

void page_read(const char *filename, char *sectorbuf, char *sparebuf, int ppn, int *reads){
	char pagebuf[PAGE_SIZE];
	flashfp = fopen(filename, "rb");
	if (!flashfp){
		fprintf(stderr, "opening flash memory file error\n");
		exit(EXIT_FAILURE);
	}
	dd_read(ppn, pagebuf);
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
    memcpy(sparebuf, pagebuf + SECTOR_SIZE, SPARE_SIZE);
	(*reads)++;
	fclose(flashfp);
	if (memcmp(sectorbuf, "\xFF", 1) != 0 || memcmp(sparebuf, "\xFF", 1) != 0){
		for (int i = 0; i < SECTOR_SIZE; i++){
			if (sectorbuf[i] == '\xFF')
				break;
			printf("%c", sectorbuf[i]);
		}
		printf(" ");
		for (int i = 0; i < SPARE_SIZE; i++){
			if (sparebuf[i] == '\xFF')
				break;
			printf("%c", sparebuf[i]);
		}
		printf("\n");
	}
}

void block_erase(const char *filename, char *blockbuf, int pbn, int *erases){
	flashfp = fopen(filename, "rb+");
	if (!flashfp){
		fprintf(stderr, "opening flash memory file error\n");
		exit(EXIT_FAILURE);
	}
	dd_erase(pbn);
	(*erases)++;
	fclose(flashfp);
}

void write_mapping(const char *filename, int ppn, char *blockbuf, const char *sectordata, const char *sparedata, int *reads, int *writes, int *erases) {
	int block_num = ppn / PAGE_NUM;
    int non_empty = 0;
	char pagebuf[PAGE_SIZE];
    flashfp = fopen(filename, "rb");
    if (!flashfp) {
       	fprintf(stderr, "opening flash memory file error\n");
       	exit(EXIT_FAILURE);
    }
    dd_read(ppn, pagebuf);
	(*reads)++;
    fclose(flashfp);

   	if (pagebuf[0] != '\xFF')
       	non_empty = 1;
   	if (non_empty){
   		for (int i = 0; i < PAGE_NUM; i++) {
   			char temp_pagebuf[PAGE_SIZE];
		flashfp = fopen(filename, "rb");
		if(!flashfp) {
			fprintf(stderr, "opening flash memory file error\n");
               exit(EXIT_FAILURE);
		}
		dd_read(i, temp_pagebuf);
		(*reads)++;
		fclose(flashfp);
		if (temp_pagebuf[0] != '\xFF') {
		fprintf(stderr, "in-place update error\n");
		exit(EXIT_FAILURE);
		}
    	}
        for (int i = 0; i < PAGE_NUM; i++) {
        	if (i != ppn % PAGE_NUM) {
        		flashfp = fopen(filename, "rb");
           		if (!flashfp) {
               		fprintf(stderr, "opening flash memory file error\n");
               		exit(EXIT_FAILURE);
           		}
           		dd_read(block_num * PAGE_NUM + i, pagebuf);
				(*reads)++;
				fclose(flashfp);
				page_write(filename, pagebuf, i, pagebuf, pagebuf + SECTOR_SIZE, writes);
			}
		}
        block_erase(filename, blockbuf, block_num, erases);
        page_write(filename, pagebuf, ppn, sectordata, sparedata, writes);
        for (int i = 0; i < PAGE_NUM; i++) {
           	if (i != ppn % PAGE_NUM) {
               	char recover_pagebuf[PAGE_SIZE];
               	flashfp = fopen(filename, "rb");
               	if (!flashfp) {
               		fprintf(stderr, "opening flash memory file error\n");
               		exit(EXIT_FAILURE);
               	}
               	dd_read(i, recover_pagebuf);
				(*reads)++;
				fclose(flashfp);
               	page_write(filename, recover_pagebuf, block_num * PAGE_NUM + i, recover_pagebuf, recover_pagebuf + SECTOR_SIZE, writes);
           	}
		}
		block_erase(filename, blockbuf, 0, erases);
    }
	else
		page_write(filename, pagebuf, ppn, sectordata, sparedata, writes);
}

int main(int argc, char *argv[])
{
	char sectorbuf[SECTOR_SIZE];
	char blockbuf[BLOCK_SIZE];
	char sparebuf[SPARE_SIZE];
	int pagereads = 0, pagewrites = 0, blockerases = 0;	
	
	if (argc < 4) {
		printf("Usage : %s <operation> <flashfile> <additional arguments>\n", argv[0]);
		return EXIT_FAILURE;
	}
	char *operation = argv[1];
	char *flashfile = argv[2];
	if (strcmp(operation, "c") == 0){
		if (argc != 4){
			printf("Usage: %s c <flashfile> <#blocks>\n", argv[0]);
			return EXIT_FAILURE;
		}
		int num_blocks = atoi(argv[3]);
		fm_create(flashfile, num_blocks, blockbuf);
	}
	else if (strcmp(operation, "w") == 0){
		if (argc != 6){
			printf("Usage: %s w <flashfile> <ppn> <sectordata> <sparedata>\n", argv[0]);
			return EXIT_FAILURE;
		}
		int ppn = atoi(argv[3]);
		const char *sectordata = argv[4];
		const char *sparedata = argv[5];
		write_mapping(flashfile, ppn, blockbuf, sectordata, sparedata, &pagereads, &pagewrites, &blockerases);
		printf("#pagereads=%d #pagewrites=%d #blockerases=%d\n", pagereads, pagewrites, blockerases);
	}
	else if (strcmp(operation, "r") == 0) {
		if (argc != 4){
			printf("Usage: %s r <flashfile> <ppn>\n", argv[0]);
			return EXIT_FAILURE;
		}
		int ppn = atoi(argv[3]);
		page_read(flashfile, sectorbuf, sparebuf, ppn, &pagereads);
	}
	else if (strcmp(operation, "e") == 0) {
		if (argc != 4) {
			printf("Usage: %s e <flashfile> <pbn>\n", argv[0]);
			return EXIT_FAILURE;
		}
		int pbn = atoi(argv[3]);
		block_erase(flashfile, blockbuf, pbn, &blockerases);
	}
	else {
		printf("Invalid operation\n");
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
