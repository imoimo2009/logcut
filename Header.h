#pragma once
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <direct.h>


#define OPT_HEAD			'-'
#define OPT_HEAD2			'/'
#define OPT_GREP			'g'
#define OPT_TAIL			't'
#define OPT_OUT				'o'
#define OPT_VIEW			'v'

#define OUT_FILENAME		"out.txt"

#define TAIL_LINES			100

#define STRING_MAX			256
#define LINE_LENGTH			2048

enum eMode{
	MODE_GREP = 1,
	MODE_TAIL
};

struct sBuffer {
	char s[LINE_LENGTH];
	sBuffer *next;
};
