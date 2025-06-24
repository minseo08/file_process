#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "student.h"

int readPage(FILE *fp, char *pagebuf, int pagenum){
        fseek(fp, FILE_HEADER_SIZE + (pagenum * PAGE_SIZE), SEEK_SET);
        return fread(pagebuf, PAGE_SIZE, 1, fp);
}

int getRecFromPagebuf(const char *pagebuf, char *recordbuf, int recordnum) {
	short num_records = *(short *)pagebuf;
	short offset;
	if (recordnum >= num_records) {
		return 0;
	}
	offset = *(short *)(pagebuf + 8 + 2 * recordnum);
	short temp;
	if (recordnum == 0) {
		temp = PAGE_HEADER_SIZE;
	} 
	else {
		temp = *(short *)(pagebuf + 8 + 2 * (recordnum - 1));
	}
	short record_size = offset - temp;
	memcpy(recordbuf, pagebuf + temp, record_size);
	recordbuf[record_size] = '\0';
	return 1;
}

void unpack(const char *recordbuf, STUDENT *s) {
	char field_buffer[50];
	int field_index = 0;
	int buffer_index = 0;
	for (int i = 0; i < MAX_RECORD_SIZE; i++) {
		if (recordbuf[i] == '#') {
			field_buffer[buffer_index] = '\0';
			switch (field_index) {
				case 0:
					strncpy(s->id, field_buffer, sizeof(s->id) - 1);
					break;
				case 1:
					strncpy(s->name, field_buffer, sizeof(s->name) - 1);
					break;
				case 2:
					strncpy(s->dept, field_buffer, sizeof(s->dept) - 1);
					break;
				case 3:
					strncpy(s->year, field_buffer, sizeof(s->year) - 1);
					break;
				case 4:
					strncpy(s->addr, field_buffer, sizeof(s->addr) - 1);
					break;
				case 5:
					strncpy(s->phone, field_buffer, sizeof(s->phone) - 1);
					break;
				case 6:
					strncpy(s->email, field_buffer, sizeof(s->email) - 1);
					break;
				default:
					break;
			}
			buffer_index = 0;
			field_index++;
		} 
		else {
			field_buffer[buffer_index++] = recordbuf[i];
		}
	}
}

int writePage(FILE *fp, const char *pagebuf, int pagenum){
	fseek(fp, FILE_HEADER_SIZE + (pagenum * PAGE_SIZE), SEEK_SET);
	return fwrite(pagebuf, PAGE_SIZE, 1, fp);
}

void writeRecToPagebuf(char *pagebuf, const char *recordbuf) {
	short num_records = *(short *)pagebuf;
	short offset;
	if (num_records == 0) {
		offset = PAGE_HEADER_SIZE;
	}
	else {
		offset = *(short *)(pagebuf + 8 + 2 * (num_records - 1));
	}
	memcpy(pagebuf + offset, recordbuf, strlen(recordbuf));
	offset += strlen(recordbuf);
	num_records++;
	*(short *)pagebuf = num_records;
	*(short *)(pagebuf + 8 + 2 * (num_records - 1)) = offset;
}

void pack(char *recordbuf, const STUDENT *s){
    sprintf(recordbuf, "%s#%s#%s#%s#%s#%s#%s#", s->id, s->name, s->dept, s->year, s->addr, s->phone, s->email);
}

int readFileHeader(FILE *fp, char *headerbuf){
	fseek(fp, 0, SEEK_SET);
	return fread(headerbuf, FILE_HEADER_SIZE, 1, fp);
}

int writeFileHeader(FILE *fp, const char *headerbuf){
	fseek(fp, 0, SEEK_SET);
	return fwrite(headerbuf, FILE_HEADER_SIZE, 1, fp);
}

void printSearchResult(const STUDENT *s, int n)
{
	int i;
	printf("#Records = %d\n", n);
	for(i=0; i<n; i++)
	{
		printf("%s#%s#%s#%s#%s#%s#%s\n", s[i].id, s[i].name, s[i].dept, s[i].year, s[i].addr, s[i].phone, s[i].email);
	}
}

int searchByID(FILE *fp, char *keyval, char *recordbuf, int *pagenum, int *recordnum) {
	char *id_val = strchr(keyval, '=') + 1;
	char pagebuf[PAGE_SIZE];
	STUDENT s;
	int pageNumber = 0;
	int recordNumber = 0;
	char headerbuf[FILE_HEADER_SIZE];
	if(!readFileHeader(fp, headerbuf)){
		printf("read fileheader error\n");
		return 0;
	}
	short numPages = *(short *)(headerbuf);
	while(pageNumber < numPages){
		if(!readPage(fp, pagebuf, pageNumber)){
			return 0;
		}
		while (getRecFromPagebuf(pagebuf, recordbuf, recordNumber)){
			if(recordbuf[0] == '*'){
				recordNumber++;
				continue;
			}
			memset(&s, 0, sizeof(STUDENT));
			unpack(recordbuf, &s);
			if(strcmp(s.id, id_val) == 0){
				printSearchResult(&s, 1);
				*pagenum = pageNumber;
				*recordnum = recordNumber;
				return 1;
			}
			recordNumber++;
		}
		pageNumber++;
		recordNumber = 0;
		if (pageNumber >= numPages)
			break;
	}
	return 0;
}

void delete(FILE *fp, char *keyval) {
	char headerbuf[FILE_HEADER_SIZE];
	char pagebuf[PAGE_SIZE];
	char recordbuf[MAX_RECORD_SIZE];
	int pageNumber, recordNumber;
	int last_deleted_page, last_deleted_record;

	if(!readFileHeader(fp, headerbuf)){
		printf("read fileheader error\n");
		return;
	}
	if(!searchByID(fp, keyval, recordbuf, &pageNumber, &recordNumber)){
		printf("Record with ID %s not found.\n", keyval);
		return;
	}

	last_deleted_page = *(short *)(headerbuf + 8);
	last_deleted_record = *(short *)(headerbuf + 10);
	sprintf(recordbuf, "*%02d%02d", last_deleted_page, last_deleted_record);

	if(!readPage(fp, pagebuf, pageNumber)){
		printf("read page error\n");
		return;
	}
	int offset = PAGE_HEADER_SIZE;
	for(int i = 0; i < recordNumber; i++){
		offset = *(short *)(pagebuf + 8 + 2 * i);
	}
	memcpy(pagebuf + offset, recordbuf, strlen(recordbuf));
	writePage(fp, pagebuf, pageNumber);

	memcpy(pagebuf + offset, recordbuf, strlen(recordbuf));
	writePage(fp, pagebuf, pageNumber);

	*(short *)(headerbuf + 8) = pageNumber;
	*(short *)(headerbuf + 10) = recordNumber;
	writeFileHeader(fp, headerbuf);
}

void search(FILE *fp, FIELD f, char *keyval)
{
	char pagebuf[PAGE_SIZE];
	char recordbuf[MAX_RECORD_SIZE];
	char headerbuf[FILE_HEADER_SIZE];
	STUDENT s;
	int pageNumber = 0;
	int recordNumber = 0;
	int found = 0;
	STUDENT searchResults[MAX_RECORD_SIZE];
	
	if (!readFileHeader(fp, headerbuf)){
		printf("read fileheader error\n");
		return;
	}
	short numPages = *(short *)(headerbuf);
	while (pageNumber < numPages){
		if (!readPage(fp, pagebuf, pageNumber)){
			printf("read page error\n");
			return;
		}
		while (getRecFromPagebuf(pagebuf, recordbuf, recordNumber)){
			if (recordbuf[0] == '*'){
				recordNumber++;
				continue;
			}
			unpack(recordbuf, &s);
			char *searchKey;
			switch (f){
				case ID:
					searchKey = s.id;
					break;
				case NAME:
					searchKey = s.name;
					break;
				case DEPT:
					searchKey = s.dept;
					break;
				case YEAR:
					searchKey = s.year;
					break;
				case ADDR:
					searchKey = s.addr;
					break;
				case PHONE:
					searchKey = s.phone;
					break;
				case EMAIL:
					searchKey = s.email;
					break;
			}
			if (strcmp(searchKey, keyval) == 0){
				searchResults[found] = s;
				found++;
			}
			recordNumber++;
		}
		pageNumber++;
		recordNumber = 0;
		if (pageNumber >= numPages)
			break;
	}
	printSearchResult(searchResults, found);
	if (!found){
		printf("no result found\n");
	}
}

void insert(FILE *fp, const STUDENT *s){
	short num_pages, num_records;
	char headerbuf[FILE_HEADER_SIZE];
	char pagebuf[PAGE_SIZE];
	char recordbuf[RECORD_SIZE];
	int pageNumber, recordNumber, found = 0;
	int last_deleted_page, last_deleted_record;
	int delete_offset, next_delete_page, next_delete_record;

	if (!readFileHeader(fp, headerbuf) || *(short *)(headerbuf) <= 0){
		num_pages = 0;
		last_deleted_page = -1;
		last_deleted_record = -1;
		*(short *)(headerbuf + 8) = -1;
		*(short *)(headerbuf + 10) = -1;
	}
	else {
		num_pages = *(short *)(headerbuf);
		last_deleted_page = *(short *)(headerbuf + 8);
		last_deleted_record = *(short *)(headerbuf + 10);
	}
	pack(recordbuf, s);
	int record_size = strlen(recordbuf);

	while (last_deleted_page != -1 && last_deleted_record != -1){
		if (readPage(fp, pagebuf, last_deleted_page)){
			int offset = PAGE_HEADER_SIZE;
			for (int i = 0; i < last_deleted_record; i++){
				offset = *(short *)(pagebuf + 8 + 2 * i);
			}
			delete_offset = offset;
			sscanf(pagebuf + offset + 1, "%02d%02d", &next_delete_page, &next_delete_record);

			int deleted_record_size = (last_deleted_record == 0 ? PAGE_SIZE : *(short *)(pagebuf + 8 + 2 * last_deleted_record)) - delete_offset;
			if (deleted_record_size >= record_size){
				memcpy(pagebuf + delete_offset, recordbuf, record_size);
				int prev_deleted_page = -1;
				int prev_deleted_record = -1;
				int current_page = *(short *)(headerbuf + 8);
				int current_record = *(short *)(headerbuf + 10);
				while (current_page != -1 && current_record != -1){
					if (current_page == last_deleted_page && current_record == last_deleted_record){
						if (prev_deleted_page != -1 && prev_deleted_record != -1){
							char prev_pagebuf[PAGE_SIZE];
							if (readPage(fp, prev_pagebuf, prev_deleted_page)) {
								int prev_offset = PAGE_HEADER_SIZE;
								for (int i = 0; i < prev_deleted_record; i++){
									prev_offset = *(short *)(prev_pagebuf + 8 + 2 * i);
								}
								if (prev_deleted_page != last_deleted_page){
									sprintf(prev_pagebuf + prev_offset + 1, "%02d%02d", next_delete_page, next_delete_record);
									writePage(fp, prev_pagebuf, prev_deleted_page);
								}
								else{
									sprintf(pagebuf + prev_offset + 1, "%02d%02d", next_delete_page, next_delete_record);
								}
							}
						}
						else {
							*(short *)(headerbuf + 8) = next_delete_page;
							*(short *)(headerbuf + 10) = next_delete_record;
							writeFileHeader(fp, headerbuf);
						}
						break;
					}
					prev_deleted_page = current_page;
					prev_deleted_record = current_record;

					char temp_pagebuf[PAGE_SIZE];
					if (readPage(fp, temp_pagebuf, current_page)){
						int temp_offset = PAGE_HEADER_SIZE;
						for (int i = 0; i < current_record; i++){
							temp_offset = *(short *)(temp_pagebuf + 8 + 2 * i);
						}
						sscanf(temp_pagebuf + temp_offset + 1, "%02d%02d", &current_page, &current_record);
					}
				}
				writePage(fp, pagebuf, last_deleted_page);
				return;
			}
			last_deleted_page = next_delete_page;
			last_deleted_record = next_delete_record;
		}
	}
	if (num_pages > 0){
		for(int pageNumber = 0; pageNumber < num_pages; pageNumber++){
			readPage(fp, pagebuf, pageNumber);
			num_records = *(short *)pagebuf;
			if (num_records < MAX_RECORDS_PER_PAGE){
				writeRecToPagebuf(pagebuf, recordbuf);
				writePage(fp, pagebuf, pageNumber);
				return;
			}
		}
	}
	memset(pagebuf, 0, PAGE_SIZE);
	writeRecToPagebuf(pagebuf, recordbuf);
	num_pages++;
	*(short *)headerbuf = num_pages;
	writeFileHeader(fp, headerbuf);
	writePage(fp, pagebuf, num_pages - 1);
}

FIELD getFieldID(char *fieldname){
	if (strcmp(fieldname, "ID") == 0)
		return ID;
	else if (strcmp(fieldname, "NAME") == 0)
		return NAME;
	else if (strcmp(fieldname, "DEPT") == 0)
		return DEPT;
	else if (strcmp(fieldname, "YEAR") == 0)
		return YEAR;
	else if (strcmp(fieldname, "ADDR") == 0)
		return ADDR;
	else if (strcmp(fieldname, "PHONE") == 0)
		return PHONE;
	else if (strcmp(fieldname, "EMAIL") == 0)
		return EMAIL;
	else
		return -1;
}

int main(int argc, char *argv[])
{
	FILE *fp;
	char headerbuf[FILE_HEADER_SIZE];
	char *record_file_name = argv[2];
	char *field_arg = argv[3];

	fp = fopen(record_file_name, "r+b");
    	if (fp == NULL){
        	fp = fopen(record_file_name, "w+b");
        	if (fp == NULL){
            		printf("Error creating file\n");
            		return 1;
        	}
	}
	if (strcmp(argv[1], "-i") == 0){
		STUDENT new_student;
		for(int i = 3; i < argc; i++){
			char *field_name = strtok(argv[i], "=");
			char *field_value = strtok(NULL, "=");
			FIELD field_id = getFieldID(field_name);
			if (field_id == -1){
				printf("Invalid field name: %s\n", field_name);
				fclose(fp);
				return 1;
			}
			switch (field_id){
				case ID:
					strcpy(new_student.id, field_value);
					break;
				case NAME:
					strcpy(new_student.name, field_value);
					break;
				case DEPT:
					strcpy(new_student.dept, field_value);
					break;
				case YEAR:
					strcpy(new_student.year, field_value);
					break;
				case ADDR:
					strcpy(new_student.addr, field_value);
					break;
				case PHONE:
					strcpy(new_student.phone, field_value);
					break;
				case EMAIL:
					strcpy(new_student.email, field_value);
					break;
				default:
					break;
			}
		}
		insert(fp, &new_student);
	}
	else if (strcmp(argv[1], "-s") == 0) {
		char *field_name = strtok(field_arg, "=");
		FIELD field_id = getFieldID(field_name);
		if (field_id == -1){
			printf("Invalid field name: %s\n", argv[3]);
			fclose(fp);
			return 1;
		}
		if (field_id != -1){
			char *field_value = strtok(NULL, "=");
			search(fp, field_id, field_value);
		}
	}
	else if (strcmp(argv[1], "-d") == 0){
		delete(fp, field_arg);
	}
	else {
		printf("Usage: %s <operation> <record_file_name> <args>\n", argv[0]);
		fclose(fp);
		return 1;
	}
	fclose(fp);
	return 0;
}

