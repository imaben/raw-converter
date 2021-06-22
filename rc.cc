#include <stdio.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "libraw/libraw.h"

#ifndef LIBRAW_WIN32_CALLS
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#endif

#ifdef LIBRAW_WIN32_CALLS
#define snprintf _snprintf
#endif

#define RC_VERSION "v0.0.1"

#define fatal(fmt, ...) do {             \
	fprintf(stderr, fmt, ##__VA_ARGS__); \
	exit(1);                             \
} while (0)

typedef struct {
	int lst;
	int verb;
	int tiff;
	int width;
	int height;
	char *file;
	char *output;
} rc_options;

void usage()
{
	printf("Raw Photo Converter "
			RC_VERSION "\n"
			"Based on LibRaw, version: %s\n"
			"Usage: \n"
			"  rc [OPTIONS] raw-file > target.ppm\n"
			"  rc [OPTIONS] raw-file | cjpeg -quality 80 > target.jpg\n"
			"Options:\n"
			"  -L      list supported cameras and exit\n"
			"  -v      Print verbose messages\n"
			"  -T      output TIFF files instead of .pgm/ppm\n"
			"  -o      Specity output file\n"
			"  -w      Specify output width\n"
			"  -h      Specify output height\n", LibRaw::version());
	exit(0);
}

int init_options(int argc, char **argv, rc_options *opts)
{
	int ch;
	while ((ch = getopt(argc, argv, "LvTo:w:h:")) != -1) {
		switch (ch) {
			case 'L':
				opts->lst = 1;
				break;
			case 'v':
				opts->verb = 1;
				break;
			case 'T':
				opts->tiff = 1;
				break;
			case 'o':
				opts->output = strdup(optarg);
				break;
			case 'w':
				opts->width = atoi(optarg);
				break;
			case 'h':
				opts->height = atoi(optarg);
				break;
			default:
				usage();
		}
	}
	opts->file = argv[argc-1];
	return 0;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		usage();
	}

	rc_options opts = {0};
	if (init_options(argc, argv, &opts) < 0) {
		fatal("Failed to parse options");
	}

	if (opts.lst) {
		const char **clist = LibRaw::cameraList();
		const char **cc = clist;
		while (*cc) {
			printf("%s\n", *cc);
			cc++;
		}
		exit(0);
	}

	if (!opts.file || !strlen(opts.file)) {
		fatal("Please specity raw file");
	}
	if (access(opts.file, F_OK) < 0) {
		fatal("File %s not found", opts.file);
	}

#define P1  raw.imgdata.idata
#define S   raw.imgdata.sizes
#define C   raw.imgdata.color
#define T   raw.imgdata.thumbnail
#define P2  raw.imgdata.other
#define OUT raw.imgdata.params

	int ret;
	LibRaw raw;
	ret = raw.open_file(opts.file);
	if (LIBRAW_SUCCESS != ret) {
		fatal("Cannot open %s: %s", opts.file, libraw_strerror(ret));
	}

	if (LIBRAW_SUCCESS != (ret = raw.unpack())) {
		fatal("Cannot unpack %s: %s", opts.file, libraw_strerror(ret));
	}

	if (opts.width > 0) {
		S.iwidth = opts.width;
	}
	if (opts.height > 0) {
		S.iheight = opts.height;
	}
	if (opts.tiff) {
		OUT.output_tiff = 1;
	}

	if (LIBRAW_SUCCESS != (ret = raw.dcraw_process())) {
		fatal("Cannot do postpocessing on %s: %s", opts.file, libraw_strerror(ret));
	}
	if (LIBRAW_SUCCESS != (ret = raw.dcraw_ppm_tiff_writer(opts.output ? opts.output : "-"))) {
		fatal("Cannot write data to stdout %s: %s", opts.file, libraw_strerror(ret));
	}
	raw.recycle();
	if (opts.output) {
		free(opts.output);
	}
	return 0;
}

