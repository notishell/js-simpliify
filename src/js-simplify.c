#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pcre.h>

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

/**
 * default value
 */
static struct Options gOptions = {
	"simplify.js"
};

/**
 * global program name
 */
static const char* gProgName = PACKAGE_TARNAME;

struct js_handler {
	const char *pattern;
	int recursion;
	int options;
	int (*handle)(int argc, char **argv, int arglen[]);
};

int js_char_to_int(char c)
{
	if (c >= '0' && c <= '9') {
		return (c - '0');
	} else if (c >= 'a' && c <= 'z') {
		return (c - 'a' + 10);
	} else if (c >= 'A' && c <= 'Z') {
		return (c - 'A' + 10);
	} else {
		return (-1);
	}
}

long js_string_to_long(const char *str, int len)
{
	long v = 0;
	if (str[1] == 'x') {
		str = str + 2;
		while (len-- > 2) {
			v = v * 16 + js_char_to_int(*str++);
		}
	} else {
		if (str[0] == '0') {
			str = str + 1;
			while (len-- > 1) {
				v = v * 8 + js_char_to_int(*str++);
			}
		} else {
			while (len-- > 0) {
				v = v * 10 + js_char_to_int(*str++);
			}
		}
	}
	return (v);
}

int js_long_to_string(long v, char *str)
{
	return (sprintf(str, "%ld", v));
}

long js_calculate(long a, long b, char opt)
{
	long c;

	switch (opt) {
		case '+':
			c = a + b;
			break;
		case '-':
			c = a + b;
			break;
		case '*':
			c = a * b;
			break;
		case '&':
			c = a & b;
			break;
		case '|':
			c = a | b;
			break;
		case '%':
			c = a % b;
			break;
		default:
			return (-1);
	}

	return (c);
}

int js_calculate2num_handle(int argc, char **argv, int arglen[])
{
	int len;
	long a, b, c;
	char buffer[64];

	if (argc != 4) {
		return (-1);
	}

	a = js_string_to_long(argv[1], arglen[1]);
	b = js_string_to_long(argv[3], arglen[3]);
	c = js_calculate(a, b, argv[2][0]);

	memset(argv[0], ' ', arglen[0]);
	len = js_long_to_string(c, buffer);
	memcpy(argv[0], buffer, strlen(buffer));
	return (0);
}

int js_calculate3num_handle(int argc, char **argv, int arglen[])
{
	int len;
	long a, b, c, d;
	char buffer[64];

	if (argc != 6) {
		return (-1);
	}

	a = js_string_to_long(argv[1], arglen[1]);
	b = js_string_to_long(argv[3], arglen[3]);
	c = js_string_to_long(argv[5], arglen[5]);
	d = js_calculate(a, b, argv[2][0]);
	d = js_calculate(d, c, argv[4][0]);

	memset(argv[0], ' ', arglen[0]);
	len = js_long_to_string(d, buffer);
	memcpy(argv[0], buffer, strlen(buffer));
	return (0);
}

void js_decrypt_string(char *p, int len, int a, int b) {
	int i, j, u = a + b;

	for (i = 0, j = len - 1; i < j; i++, j--) {
		p[i] ^= p[j] ^= p[i] ^= p[j];
	}

	p[0] = '"';
	for (i = 1, j = 1; i < len - 1; i++) {
		if ((i % u) <= a && (i % u) >= 1) {
			p[j++] = p[i];
		}
	}

	for (p[j++] = '"'; j < len; j++) {
		p[j] = ' ';
	}
}

int js_decrypt_string_handle(int argc, char **argv, int arglen[])
{
	int len;
	long a, b, c, d;

	if (argc != 4) {
		return (-1);
	}

	a = js_string_to_long(argv[2], arglen[2]);
	b = js_string_to_long(argv[3], arglen[3]);

	js_decrypt_string(argv[1], arglen[1], a, b);


	memcpy(argv[0], argv[1], arglen[1]);
	memset(argv[0] + arglen[1], ' ', arglen[0] - arglen[1]);
	return (0);
}

int js_replace_string_to_dot_handle(int argc, char **argv, int arglen[])
{
	int i;

	if (argc != 1) {
		return (-1);
	}

	argv[0][0] = '.';
	for (i = 1; i < arglen[0]; i++) {
		switch (argv[0][i]) {
			case '\"':
			case '\'':
			case '[':
			case ']':
				argv[0][i] = ' ';
				break;
			default:
				break;
		}
	}

	return (0);
}

int js_simplify_for_1_handle(int argc, char **argv, int arglen[])
{
	int i;
	char *tmp = NULL;

	if (argc != 5) {
		return (-1);
	}

	tmp = (char *) malloc(arglen[0]);
	if (tmp == NULL) {
		return (-1);
	}
	memset(tmp, 0, arglen[0]);

	memcpy(tmp + strlen(tmp), argv[1], arglen[1]);
	memcpy(tmp + strlen(tmp), argv[3], arglen[3]);
	memcpy(tmp + strlen(tmp), argv[2], arglen[2]);
	memcpy(tmp + strlen(tmp), argv[4], arglen[4]);

	memcpy(argv[0], tmp, strlen(tmp));
	memset(argv[0] + strlen(tmp), ' ', arglen[0] - strlen(tmp));

	free(tmp);

	return (0);
}

int js_simplify_for_2_handle(int argc, char **argv, int arglen[])
{
	int i;
	char *tmp = NULL;

	if (argc != 5) {
		return (-1);
	}

	tmp = (char *) malloc(arglen[0]);
	if (tmp == NULL) {
		return (-1);
	}
	memset(tmp, 0, arglen[0]);

	memcpy(tmp + strlen(tmp), argv[1], arglen[1]);
	memcpy(tmp + strlen(tmp), argv[4], arglen[4]);
	memcpy(tmp + strlen(tmp), argv[3], arglen[3]);
	memcpy(tmp + strlen(tmp), argv[2], arglen[2]);

	memcpy(argv[0], tmp, strlen(tmp));
	memset(argv[0] + strlen(tmp), ' ', arglen[0] - strlen(tmp));

	free(tmp);

	return (0);
}

int js_simplify_switch_1_handle(int argc, char **argv, int arglen[])
{
	int i;
	char *tmp = NULL;

	if (argc != 3) {
		return (-1);
	}

	tmp = (char *) malloc(arglen[0]);
	if (tmp == NULL) {
		return (-1);
	}
	memset(tmp, 0, arglen[0]);

	memcpy(tmp + strlen(tmp), argv[1], arglen[1]);
	memcpy(tmp + strlen(tmp), argv[2], arglen[2]);

	memcpy(argv[0], tmp, strlen(tmp));
	memset(argv[0] + strlen(tmp), ' ', arglen[0] - strlen(tmp));

	free(tmp);

	return (0);
}

int js_simplify_switch_2_handle(int argc, char **argv, int arglen[])
{
	int i;
	char *tmp = NULL;

	if (argc != 4) {
		return (-1);
	}

	tmp = (char *) malloc(arglen[0]);
	if (tmp == NULL) {
		return (-1);
	}
	memset(tmp, 0, arglen[0]);

	memcpy(tmp + strlen(tmp), argv[1], arglen[1]);
	memcpy(tmp + strlen(tmp), argv[2], arglen[2]);
	memcpy(tmp + strlen(tmp), argv[3], arglen[3]);

	memcpy(argv[0], tmp, strlen(tmp));
	memset(argv[0] + strlen(tmp), ' ', arglen[0] - strlen(tmp));

	free(tmp);

	return (0);
}

int js_simplify_while_1_handle(int argc, char *argv[], int arglen[])
{
	int i;
	char *tmp = NULL;

	if (argc != 2) {
		return (-1);
	}

	tmp = (char *) malloc(arglen[0]);
	if (tmp == NULL) {
		return (-1);
	}
	memset(tmp, 0, arglen[0]);

	memcpy(tmp + strlen(tmp), argv[1], arglen[1]);

	memcpy(argv[0], tmp, strlen(tmp));
	memset(argv[0] + strlen(tmp), ' ', arglen[0] - strlen(tmp));

	free(tmp);

	return (0);
}

int js_simplify_while_2_handle(int argc, char *argv[], int arglen[])
{
	int i;
	char *tmp = NULL;

	if (argc != 3) {
		return (-1);
	}

	tmp = (char *) malloc(arglen[0]);
	if (tmp == NULL) {
		return (-1);
	}
	memset(tmp, 0, arglen[0]);

	memcpy(tmp + strlen(tmp), argv[1], arglen[1]);
	memcpy(tmp + strlen(tmp), argv[2], arglen[2]);

	memcpy(argv[0], tmp, strlen(tmp));
	memset(argv[0] + strlen(tmp), ' ', arglen[0] - strlen(tmp));

	free(tmp);

	return (0);
}

int js_simplify_if_1_handle(int argc, char *argv[], int arglen[])
{
	int i;
	char *tmp = NULL;

	if (argc != 2) {
		return (-1);
	}

	tmp = (char *) malloc(arglen[0]);
	if (tmp == NULL) {
		return (-1);
	}
	memset(tmp, 0, arglen[0]);

	memcpy(tmp + strlen(tmp), argv[1], arglen[1]);

	memcpy(argv[0], tmp, strlen(tmp));
	memset(argv[0] + strlen(tmp), ' ', arglen[0] - strlen(tmp));

	free(tmp);

	return (0);
}

struct js_handler handlers[] = {
	{
		/**
		 *
		 */
		"\\(([1-9][0-9]*|0x[0-9a-fA-F]+|0[0-9]*)[ ]*([+\\-*/&\\|%])[ ]*([1-9][0-9]*|0x[0-9a-fA-F]+|0[0-9]*)\\)",
		1,
		0,
		js_calculate2num_handle
	},
	{
		/**
		 *
		 */
		"\\(([1-9][0-9]*|0x[0-9a-fA-F]+|0[0-9]*)[ ]*([+\\-*/&\\|%])[ ]*([1-9][0-9]*|0x[0-9a-fA-F]+|0[0-9]*)[ ]*([+\\-*/&\\|%])[ ]*([1-9][0-9]*|0x[0-9a-fA-F]+|0[0-9]*)\\)",
		1,
		0,
		js_calculate3num_handle
	},
	{
		/**
		 *
		 */
		"\\b[a-zA-Z][a-zA-Z0-9]\\('([a-zA-Z0-9/\\- \\[\\],\\(:\{]+)',[ ]*([0-9]+),[ ]*([0-9]+)\\)",
		1,
		0,
		js_decrypt_string_handle
	},
	{
		/**
		 *
		 */
		"\\[[ ]*(?:(?:\"[a-zA-Z0-9:/]+\")|(?:\'[a-zA-Z0-9:/]+\'))[ ]*\\]",
		1,
		0,
		js_replace_string_to_dot_handle
	},
	{
		/**
		 *
		 */
		"var[^;]+;[^f;]+for[^+]+\\+\\+[^+]+\\+[^+]+\\+{3,3}[^\\{]+\\{[^\\{]+\\{(\\s*[^;]+;)[^\\{]+\\{(\\s*[^;]+;)[^\\{;]+\\{(\\s*[^;]+;)[^\\{]+\\{(\\s*[^;]+;)\\s*\\}\\s*\\}",
		1,
		0,
		js_simplify_for_1_handle
	},
	{
		/**
		 *
		 */
		"var[^;]+;[^f;]+for[^+]+\\+\\+[^+]+\\+[^+]+\\+{3,3}[^\\{]+\\{[^\\{]+\\{(\\s*[^;]+;)[^\\{]+\\{(\\s*[^;]+;)[^\\{]+\\{(\\s*[^;]+;)[^\\{]+\\{(\\s*[^;]+;)\\s*\\}\\s*\\}",
		1,
		0,
		js_simplify_for_2_handle
	},
	{
		/**
		 *
		 */
		"var[^;\\{]+;\\s*var[^;]+;\\s*switch\\s*[^\\{;]+\\{\\s*case[^:]+:(\\s*[^;]+;)\\s*case[^:]+:(\\s*[^;]+;)\\s*\\w+[^:]+:\\s*[^;]+;\\s*\\}",
		1,
		0,
		js_simplify_switch_1_handle
	},
	{
		/**
		 *
		 */
		"var[^;\\{]+;\\s*var[^;]+;\\s*switch\\s*[^\\{;]+\\{\\s*case[^:]+:(\\s*[^;]+;)\\s*case[^:]+:(\\s*[^;]+;)\\s*\\w+[^:]+:(\\s*[^;]+;)\\s*[^;]+;\\s*\\}",
		1,
		0,
		js_simplify_switch_2_handle
	},
	{
		/**
		 *
		 */
		"var\\s*[^;\\{]+;\\s*while\\s*\\(\\s*![^\\{;]+\\{\\s*if\\s*\\([^\\)\\{;]+\\)\\s*\\{(\\s*[^;]+;[^\\w]*)[^;]+;\\s*[^;]+;\\s*\\}\\s*[^;]+;\\s*\\}",
		1,
		0,
		js_simplify_while_1_handle
	},
	{
		/**
		 *
		 */
		"var\\s*[^;\\{]+;\\s*while[^\\{]+\\{\\s*if[^\\{]+\\{(\\s*[^;]+;)\\s*[^;]+;\\s*continue\\s*;\\s*\\}\\s*[^\\}]+\\}\\s*[^\\}]+\\}",
		1,
		0,
		js_simplify_while_1_handle
	},
	{
		/**
		 *
		 */
		"var\\s*[^;\\{]+;\\s*while\\s*\\(\\s*![^\\{;]+\\{\\s*if\\s*\\([^\\)\\{;]+\\)\\s*\\{(\\s*[^;]+;)\\s*[^;]+;(\\s*[^;]+;)\\s*[^;]+;\\s*\\}\\s*[^;]+;\\s*\\}",
		1,
		0,
		js_simplify_while_2_handle
	},
	{
		/**
		 *
		 */
		"if\\s*\\(!\\d\\s*\\)\\s+\\{[^\\}]+\\}\\s*else\\s*\\{[^\\}]+\\}\\s*if\\s*\\([^\\{]+\\{\\s*[^\\};]+;\\s*\\}\\s*if\\s*\\([^\\{]+\\{(\\s*[^\\};]+;)\\s*\\}\\s*else\\s*(?:\\{[^\\}\\{]+\\}|\\{[^\\}\\{]+\\{[^\\}]+\\}\\s*\\})",
		1,
		0,
		js_simplify_if_1_handle
	},
};

void js_form(char *output)
{
	int word, blank;
	char *a = output, *b = output, last;

	word = blank = 0;
	while (*a != '\0') {
		if ((*a >= 'a' && *a <= 'z') || (*a >= 'A' && *a <= 'Z') || (*a >= '0' && *a <= '9') || *a == '_') {
			if (blank != 0) {
				b++;
			}
			word++;
			blank = 0;
			*b++ = *a;
		} else {
			if (*a == ' ' || *a == '\r' || *a == '\n' || *a == '\v' || *a == '\t') {
				*b = ' ';
				if (word != 0) {
					blank = 1;
				}
			} else {
				*b++ = *a;
				blank = 0;
			}
			word = 0;
		}
		a++;
	}
	*b = '\0';
}

int js_simplify(char *input, char *output)
{
	char **array = NULL;
	const char *errptr = NULL;
	int i, j, r, len, offset, result = -1, erroffset = 0, vsz = 0, *lenlist = NULL, *ovector = NULL;
	pcre *pc = NULL;

	strcpy(output, input);

	for (i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
		j = strlen(handlers[0].pattern);
		if (vsz < j) {
			vsz = j;
		}
	}

	ovector = (int *) malloc(sizeof(int)*vsz);
	if (ovector == NULL) {
    	fprintf(stderr, "ERROR: malloc(sizeof(int)*%d) failed: %s", vsz, strerror(errno));
        goto bail;
	}
	memset(ovector, 0 , sizeof(int) * vsz);

	array = (char **) malloc(sizeof(char *)*vsz);
	if (array == NULL) {
    	fprintf(stderr, "ERROR: malloc(sizeof(char *)*%d) failed: %s", vsz, strerror(errno));
        goto bail;
	}
	memset(array, 0 , sizeof(char *)*vsz);

	lenlist = (int *) malloc(sizeof(int)*vsz);
	if (lenlist == NULL) {
    	fprintf(stderr, "ERROR: malloc(sizeof(int)*%d) failed: %s", vsz, strerror(errno));
        goto bail;
	}
	memset(lenlist, 0 , sizeof(int) * vsz);

	len = strlen(output);
	for (i = 0; i < sizeof(handlers) / sizeof(handlers[0]); i++) {
		if (pc) {
			pcre_free(pc);
		}

		pc = pcre_compile(handlers[i].pattern, handlers[i].options, &errptr, &erroffset, NULL);
		if (pc == NULL) {
	    	fprintf(stderr, "ERROR: pcre_compile(%s) failed: %s", handlers[i].pattern, errptr);
			goto bail;
		}

		offset = 0;
		while (offset < len) {
			r = pcre_exec(pc, NULL, output, len, offset, 0, ovector, vsz);
			if (r < 0) {
				if (r != PCRE_ERROR_NOMATCH) {
					fprintf(stderr, "PCRE_ERROR_NOMATCH\n");
					goto bail;
				} else {
					if (offset == 0) {
						break;
					} else {
						offset = 0;
					}
				}
			} else {
				for (j = 0; j < r; j++) {
					array[j] = output + ovector[2*j];
					lenlist[j] = ovector[2*j+1] - ovector[2*j];
				}
				if (0 != handlers[i].handle(r, array, lenlist)) {
					goto bail;
				}
				offset = ovector[1];
				memset(ovector, 0 , sizeof(int) * vsz);
			}
		}
	}

	js_form(output);
	len = strlen(output);

	result = 0;

bail:
	if (pc) {
		pcre_free(pc);
	}

	if (lenlist) {
		free(lenlist);
	}

	if (array) {
		free(array);
	}

	if (ovector) {
		free(ovector);
	}

	if (result != 0) {
		fprintf(stderr, "failed\n");
		return (-1);
	}

	return (len);
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
	int i, fd;
	char tmp[512];

	fd = open(path, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);
	if (fd >= 0) {
		return (fd);
	}

	memcpy(tmp, path, sizeof(tmp));
	for (i = strlen(tmp) - 1; i >= 0; i--) {
		if ('/' == tmp[i]) {
			tmp[i] = '\0';
			break;
		}
	}
	if (js_create_directroy(tmp) == 0) {
		return (open(path, O_WRONLY | O_BINARY | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO));
	}

	return (-1);
}

int js_simplify_file(char *input, char *output)
{
	ssize_t length;
	int i = -1, o = -1, s = 0, result = -1;
    char *iPtr = NULL, *oPtr = NULL;

    i = open(input, O_RDONLY | O_BINARY);
    if (i < 0) {
    	fprintf(stderr, "ERROR: open(%s, RB) failed: %s", input, strerror(errno));
        goto bail;
    }

    length = lseek(i, 0L, SEEK_END);

    iPtr = (char *) malloc(length);
    if (iPtr == NULL) {
    	fprintf(stderr, "ERROR: malloc(%d) failed: %s", (int) length, strerror(errno));
        goto bail;
    }

    oPtr = (char *) malloc(length);
    if (oPtr == NULL) {
    	fprintf(stderr, "ERROR: malloc(%d) failed: %s", (int) length, strerror(errno));
        goto bail;
    }

    lseek(i, 0L, SEEK_SET);
    if (read(i, iPtr, length) != length) {
    	fprintf(stderr, "ERROR: read(%d) failed: %s", i, strerror(errno));
        goto bail;
    }

    s = js_simplify(iPtr, oPtr);
    if (s < 0) {
    	fprintf(stderr, "ERROR: JS simplify failed");
        goto bail;
    }

	o = js_create_file(output);
	if (o < 0) {
    	fprintf(stderr, "ERROR: create(%s) failed: %s", output, strerror(errno));
        goto bail;
	}

	s = write(o, oPtr, s);
    if (s < 0) {
    	fprintf(stderr, "ERROR: write(%d) failed: %s", o, strerror(errno));
        goto bail;
    }

    result = 0;

bail:
	if (i >= 0) {
		close(i);
	}

	if (o >= 0) {
		close(o);
	}

	if (iPtr) {
		free(iPtr);
	}

	if (oPtr) {
		free(oPtr);
	}

	return (result);
}

/*
 * Show usage.
 */
void usage(void)
{
    fprintf(stderr, "Copyright (C) " PACKAGE_VERSION " notishell<at>gmail.com\n\n");
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
    int ic, us = 0, result = 0;

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
        us = 1;
    }

    if (us) {
        usage();
        return (-1);
    }

    if (optind < argc) {
        result = js_simplify_file(argv[optind++], gOptions.outFile);
    }

    return (result != 0);
}
