#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) 
{
	FILE *file;
    	long file_size, delete_size;
    	long start_offset = atoi(argv[1]);
    	long bytes_count = atoi(argv[2]);

    	file = fopen(argv[3], "r+");
    	if (file == NULL) {
        	fprintf(stderr, "open error\n");
        	return 1;
    	}

    	fseek(file, 0, SEEK_END);
    	file_size = ftell(file);

	if (bytes_count > 0)
	{
		start_offset++;
		long end_offset = start_offset + bytes_count;
		if (start_offset < 0)
			start_offset = 0;
		if (end_offset > file_size)
			end_offset = file_size;
		delete_size = end_offset - start_offset;

    		fseek(file, end_offset, SEEK_SET);
		char temp[file_size - end_offset + 1];
		fread(temp, file_size - end_offset, 1, file);
		fseek(file, start_offset, SEEK_SET);
		fwrite(temp, file_size - end_offset, 1, file);
	}
	else
	{
		long end_offset = start_offset - abs(bytes_count);
		if (end_offset < 0)
			end_offset = 0;
		if (start_offset > file_size)
			start_offset = file_size;
		delete_size = start_offset - end_offset;

		fseek(file, start_offset, SEEK_SET);
		char temp[file_size - start_offset + 1];
		fread(temp, file_size - start_offset, 1, file);
		fseek(file, end_offset, SEEK_SET);
		fwrite(temp, file_size - start_offset, 1, file);
	}
	fseek(file, file_size - delete_size, SEEK_SET);	
	ftruncate(fileno(file), file_size - delete_size);

    	fclose(file);
    	return 0;
}
