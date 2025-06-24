#include <stdio.h>
#include <stdlib.h>

#define BUFF_SIZE 10
int main(int argc, char *argv[]) {
	FILE *sc_file, *ds_file;
    	char buff[BUFF_SIZE];
    	size_t bytes_read;

    	sc_file = fopen(argv[1], "rb");
    	if (sc_file == NULL) {
        	fprintf(stderr, "open error for source file\n");
        	return 1;
    	}

    	ds_file = fopen(argv[2], "wb");
    	if (ds_file == NULL) {
        	fprintf(stderr, "open error for destination file\n");
        	fclose(sc_file);
        	return 1;
    	}

    	while ((bytes_read = fread(buff, 1, BUFF_SIZE, sc_file)) > 0) {
        	fwrite(buff, 1, bytes_read, ds_file);
    	}

    	fclose(sc_file);
    	fclose(ds_file);

    	return 0;
}

