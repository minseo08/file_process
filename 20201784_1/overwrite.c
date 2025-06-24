#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
	FILE *file;
	int offset;
	char *data;
	size_t data_size;

	offset = atoi(argv[1]);
	data = argv[2];
	data_size = strlen(data);

	file = fopen(argv[3], "r+");
	if (file == NULL)
	{
		fprintf(stderr, "open file error\n");
		return 1;
	}
	fseek(file, offset, SEEK_SET);
	fwrite(data, 1, data_size, file);
	fclose(file);
	
	return 0;
}
