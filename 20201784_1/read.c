#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
	FILE *file;
	
	int offset = atoi(argv[1]);
	int bytes_count = atoi(argv[2]);
	char buff[abs(bytes_count) + 1];

	file = fopen(argv[3], "rb");
	if (file == NULL)
	{
		fprintf(stderr, "open error\n");
		return 1;
	}
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);

	size_t bytes_read;
	if (bytes_count > 0)
	{
		if(offset + bytes_count >= file_size)
		{
			fseek(file, offset + 1, SEEK_SET);
			bytes_read = fread(buff, 1, offset + bytes_count - file_size, file);
		}
		else
		{
			fseek(file, offset + 1, SEEK_SET);
			bytes_read = fread(buff, 1, bytes_count, file);
		}
	}
	else
	{
		if (offset <= abs(bytes_count))
		{
			fseek(file, 0, SEEK_SET);
			bytes_read = fread(buff, 1, offset, file);
		}
		else 
		{
			fseek(file, offset + bytes_count, SEEK_SET);
			bytes_read = fread(buff, 1, abs(bytes_count), file);
		}
	}
	if (bytes_read > 0)
	{
		fwrite(buff, 1, bytes_read, stdout);
		printf("\n");
	}
	fclose(file);
	return 0;
}
