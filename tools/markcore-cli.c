
#include "markcore.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
	if (argc < 2) {
		fprintf(stderr, "Usage: %s <file.md>\n", argv[0]);
		return 1;
	}

	FILE *fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "Couldn't open file: %s\n", argv[1]);
		return 1;
	}

	if (fseek(fp, 0, SEEK_END) != 0) {
		fprintf(stderr, "Couldn't seek file: %s\n", argv[1]);
		fclose(fp);
		return 1;
	}

	long size = ftell(fp);
	if (size < 0) {
		fprintf(stderr, "Couldn't determine file size: %s\n", argv[1]);
		fclose(fp);
		return 1;
	}
	rewind(fp);

	char *buffer = malloc((size_t)size + 1);
	if (!buffer) {
		fprintf(stderr, "Out of memory reading: %s\n", argv[1]);
		fclose(fp);
		return 1;
	}

	size_t nread = fread(buffer, 1, (size_t)size, fp);
	buffer[nread] = '\0';
	fclose(fp);

	markcore_render_to_file(buffer, nread, stdout);

	free(buffer);

	return 0;
}
