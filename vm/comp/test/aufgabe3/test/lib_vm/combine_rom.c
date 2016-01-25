/* $Id: combine_rom.c 9992 2014-10-15 11:42:43Z raimue $ *
 *
 * Combine several roms into one, with the ability to specifiy
 * fixed offsets in the rom.
 *
 * Copyright (C) 2009 Stefan Potyra, Friedrich-Alexander University
 * Erlangen-Nuremberg, Department of Computer Science 3 
 * (Computer Architecture).
 *
 * Part of the lecture "virtual machines".
 */


#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <errno.h>

struct rom_s {
	const char *rom_file;
	unsigned long offset;
};

static void
usage(void)
{
	fprintf(stderr, "combine_rom -o <outfile> <offset> <rom> ..\n");
}

static unsigned long
get_offset(const char *s)
{
	if (strncmp(s, "0x", 2) == 0) {
		return strtol(s + 2, NULL, 16);
	} 

	return strtol(s, NULL, 10);
}

static int 
compare_offset(const void *_r1, const void *_r2)
{
	const struct rom_s *r1 = (const struct rom_s *)_r1;
	const struct rom_s *r2 = (const struct rom_s *)_r2;

	if (r1->offset < r2->offset) {
		return -1;
	} else if (r1->offset == r2->offset) {
		return 0;
	}
	return 1;
}

static void
append_to(FILE *fout, FILE *fin, long *offset)
{
	static char buf[4096];
	size_t ret_r;
	size_t ret_w;

	while (true) {
		ret_r = fread(buf, 1, 4096, fin);
		ret_w = fwrite(buf, 1, ret_r, fout);

		if (ret_w < ret_r) {
			perror("Problem writing to outfile.\n");
			assert(false);
		}
		
		if ((ret_r < 4096) && feof(fin)) {
			break;
		}

		if ((ret_r < 4096) && ferror(fin)) {
			perror("Error reading infile.\n");
			assert(false);
		}
	}

	*offset = ftell(fin);
}

static int
combine(const struct rom_s *roms, int nmembers, const char *outfile)
{
	int i;
	FILE *fout;
	FILE *fin;
	long cur_offset = 0;
	int ret;

	fout = fopen(outfile, "w");
	if (fout == NULL) {
		fprintf(stderr, "Could not open <%s> for writing: %s\n",
			outfile, strerror(errno));
		return EXIT_FAILURE;
	}

	for (i = 0; i < nmembers; i++) {
		assert(roms[i].rom_file != NULL);
		assert(cur_offset <= roms[i].offset);

		ret = fseek(fout, roms[i].offset, SEEK_SET);
		assert(0 <= ret);

		fin = fopen(roms[i].rom_file, "r");
		if (fin == NULL) {
			fprintf(stderr, "Could not open <%s> for reading: "
				"%s\n", roms[i].rom_file, strerror(errno));
			fclose(fout);
			return EXIT_FAILURE;
		}

		append_to(fout, fin, &cur_offset);
		ret = fclose(fin);
		assert(ret == 0);
	}

	ret = fclose(fout);
	assert(ret == 0);

	return EXIT_SUCCESS;
}

int
main(int argc, char **argv)
{
	const char *outfile = NULL;
	struct rom_s roms[10];
	int rom_members = 0;
	bool is_offset = true;

	argv++;
	while (*argv != NULL) {
		if (strcmp(*argv, "-o") == 0) {
			argv++;
			if (*argv == NULL) {
				usage();
				return EXIT_FAILURE;
			}
			outfile = *argv;
		} else if (is_offset) {
			roms[rom_members].rom_file = NULL;
			roms[rom_members].offset = get_offset(*argv);
			is_offset = false;
		} else {
			roms[rom_members].rom_file = *argv;
			rom_members++;
			assert(rom_members < 10);
			is_offset = true;
		}

		argv++;
	}

	if ((rom_members == 0) || (outfile == NULL)) {
		usage();
		return EXIT_FAILURE;
	}

	qsort(roms, rom_members - 1, sizeof(struct rom_s), compare_offset);
	return combine(roms, rom_members, outfile);
}
