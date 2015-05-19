#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>


#include "js-simplify.h"

/**
 * check if defined target name
 */
#ifndef PACKAGE_TARNAME
#define PACKAGE_TARNAME ""
#endif

/**
 * global command-line options
 */
struct Options {
    char *outFile;
};
struct Options gOptions;

/**
 * global program name
 */
static const char* gProgName = PACKAGE_TARNAME;





int is_space(char c) {
	char *s = " \r\n\t";
	while (*s != '\0') {
		if (*s == c) {
			return (1);
		}
		s++;
	}
	return (0);
}

char *js_skip_space(char *input) {
	while (is_space(*input)) {
		input++;
	}
	return (input);
}

int js_accept_const3(char *input) {
	return (-1);
	if (*input == '(') {
		input = js_skip_space(input);
		input = js_has_int(input);
	}
	return (0);
}







int js_simplify(char *input, char *output)
{
	int count;
	char *save_out = output;
	while (*input != '\0') {
		count = js_accept_const3(input);
		if (count > 0) {
			js_simplify_handl(&input, &output);
			input += count;
		} else {
			*output++ = *input++;
		}
	}
	return (output - save_out);
}

int js_create_directroy(const char *path)
{
	int i;
	char tmp[512];

	if (access(path, W_OK | F_OK) == 0) {
		return (0);
	}

	if (mkdir(path, S_IRWXU) == 0) {
		return (0);
	}

	memcpy(tmp, path, sizeof(tmp));
	for (i = strlen(tmp) - 1; i >= 0; i--) {
		if ('/' == tmp[i]) {
			tmp[i] = '\0';
			break;
		}
	}
	if (js_create_directroy(tmp) == 0) {
		return (mkdir(path, S_IRWXU));
	}
	return (-1);
}

int js_create_file(const char *path)
{
	int i;
	char tmp[512];

	if (access(path, W_OK | F_OK) == 0) {
		return (open(path, O_WRONLY | O_BINARY));
	}

	memcpy(tmp, path, sizeof(tmp));
	for (i = strlen(tmp) - 1; i >= 0; i--) {
		if ('/' == tmp[i]) {
			tmp[i] = '\0';
			break;
		}
	}
	if (js_create_directroy(tmp) == 0) {
		return (open(path, O_WRONLY | O_BINARY | O_CREAT));
	}

	return (-1);
}

int js_simplify_file(char *input, char *output) {
	int i, o, s;
    void *memPtr;
    char *iPtr, *oPtr;
	ssize_t length;

    i = open(input, O_RDONLY | O_BINARY);
    if (i < 0) {
    	fprintf(stderr, "ERROR: open(%s, RB) failed: %s", input, strerror(errno));
        return (-1);
    }

    length = lseek(i, 0L, SEEK_END);

    iPtr = (char *) malloc(length);
    oPtr = (char *) malloc(length);

    if (iPtr == NULL || oPtr == NULL) {
    	fprintf(stderr, "ERROR: malloc(%d) failed: %s", (int) length, strerror(errno));
    	return (-2);
    }

    lseek(i, 0L, SEEK_SET);
    if (read(i, iPtr, length) < 0) {
    	fprintf(stderr, "ERROR: read(%d) failed: %s", i, strerror(errno));
    	return (-3);
    }

    s = js_simplify(iPtr, oPtr);
    if (s < 0) {
    	fprintf(stderr, "ERROR: JS simplify failed");
    	return (-4);
    }

	o = js_create_file(output);
	if (o < 0) {
    	fprintf(stderr, "ERROR: create(%s) failed: %s", output, strerror(errno));
		return (-5);
	}

	s = write(o, oPtr, s);
    if (s < 0) {
    	fprintf(stderr, "ERROR: write(%d) failed: %s", o, strerror(errno));
    	return (-4);
    }

    free(iPtr);
    free(oPtr);

    close(i);
    close(o);

	return (0);
}

/*
 * Show usage.
 */
void usage(void)
{
    fprintf(stderr, "Copyright (C) 2007 The Android Open Source Project\n\n");
    fprintf(stderr, "%s: [-o out] file.js\n", gProgName);
    fprintf(stderr, "\n");
    fprintf(stderr, " -h : help\n");
    fprintf(stderr, " -o : output file\n");
}

/**
 * entry
 */
int main (int argc, char* const argv[])
{
    int ic, us = 0;

    memset(&gOptions, 0, sizeof(gOptions));

    while (1) {
        ic = getopt(argc, argv, "ho:");
        if (ic < 0)
            break;

        switch (ic) {
        case 'o':
        	gOptions.outFile = optarg;
            break;
        case 'h':
        default:
        	us = 1;
            break;
        }
    }

    if (optind == argc) {
        fprintf(stderr, "%s: no file specified\n", gProgName);
        us = 1;
    }

    if (us) {
        usage();
        return (-1);
    }

    int result = 0;
    if (optind < argc) {
        result = js_simplify_file(argv[optind++], gOptions.outFile);
    }

    return (result != 0);
}
