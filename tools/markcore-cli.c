
#include "markcore.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	if (argc < 2) {
        fprintf(stderr, "Usage: %s <file.md>\n", argv[0]);
        return 1;
    }
    
    FILE *fp = fopen(argv[1], "r");
    if (!fp) {
    	fprintf(stderr, "Couldn't open file: %s\n", argv[1]);
    	return 1;
    }
    
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);

    char *buffer = malloc(size + 1);
    fread(buffer, 1, size, fp);
    buffer[size] = '\0';
    fclose(fp);
    
    fp = fopen("output.html", "w");
    if (!fp) {
    	fprintf(stderr, "Couldn't open output file\n");
    	return 1;
    }
    
	(void)markcore_render_to_file(buffer, size, fp);
    
    free(buffer);
    
    return 0;
}