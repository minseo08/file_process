#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	FILE *file;
	char *data;
	int offset = atoi(argv[1]);
	data = argv[2];
	size_t data_size = strlen(data);

	file = fopen(argv[3], "r+");
	if (file == NULL)
	{
		fprintf(stderr, "open error\n");
		return 1;
	}
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);

	if (offset >= file_size)
		fwrite(data, 1, data_size, file);
	else
	{
		fseek(file, offset + 1, SEEK_SET);
		char temp[file_size - offset - 1];
		fread(temp, 1, file_size - offset - 1, file);
		fseek(file, offset + 1, SEEK_SET);
		fwrite(data, 1, data_size, file);
		fwrite(temp, 1, file_size - offset - 1, file);
	}
	fclose(file);
	return 0;
}
