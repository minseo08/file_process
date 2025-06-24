#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 1024
int main(int argc, char *argv[])
{
	FILE *file1, *file2, *file3;
	char buff[BUFF_SIZE];
	size_t bytes_read;

	file1 = fopen(argv[1], "rb");
	if(file1 == NULL)
	{
		fprintf(stderr, "open error for first file\n");
		return 1;
	}

	file2 = fopen(argv[2], "rb");
	if(file1 == NULL)
	{
		fprintf(stderr, "open error for second file\n");
		fclose(file1);
		return 1;
	}
	
	file3 = fopen(argv[3], "wb");
	if(file1 == NULL)
	{
		fprintf(stderr, "open error for third file\n");
		fclose(file1);
		fclose(file2);
		return 1;
	}
	while ((bytes_read = fread(buff, 1, sizeof(buff), file1)) > 0)
	{
		fwrite(buff, 1, bytes_read, file3);
	}
	rewind(file2);
	while ((bytes_read = fread(buff, 1, sizeof(buff), file2)) > 0)
	{
		fwrite(buff, 1, bytes_read, file3);
	}
	fclose(file1);
	fclose(file2);
	fclose(file3);

	return 0;
}
