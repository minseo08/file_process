#ifndef _STUDENT_H_
#define _STUDENT_H_

#define MAX_RECORD_SIZE	100		//id(8) + name(13) + department(16) + year(1) + address(20) + phone(15) +  email(20) + 7 delimeters
#define PAGE_SIZE		512		// 512 Bytes
#define PAGE_HEADER_SIZE	64		// 64 Bytes
#define FILE_HEADER_SIZE	16		// 16 Bytes
#define RECORD_SIZE (MAX_RECORD_SIZE - 7)
#define MAX_RECORDS_PER_PAGE ((PAGE_SIZE - PAGE_HEADER_SIZE) / RECORD_SIZE)
typedef enum {ID=0, NAME, DEPT, YEAR,  ADDR, PHONE, EMAIL} FIELD;

typedef struct _STUDENT
{
	char id[9];			// 학번
	char name[14];		// 이름
	char dept[17];		// 학과
	char year[2];		// 학년
	char addr[21];		// 주소
	char phone[16];		// 전화번호
	char email[21];		// 이메일 주소
} STUDENT;

#endif
