
/*
 *  CONFIG.C
 *
 *  (C) Copyright 1989-1990 by Matthew Dillon,  All Rights Reserved.
 *
 *  Extract fields from Config file
 */

#include <stdio.h>
#include <stdlib.h>

#define CTLZ	('z'&0x1F)

static char *ConfBuf = NULL;

char *
FindConfig(field)
char *field;
{
    char *str;
    short flen = strlen(field);

    if (ConfBuf == NULL) {
	FILE *fi = fopen("d:\\uupc\\uupc.rc", "r");
	if (fi) {
	    long buflen;
	    fseek(fi, 0L, 2);
	    buflen = ftell(fi);
	    fseek(fi, 0L, 0);
	    if (buflen > 0L && (ConfBuf = malloc(buflen + 1))) {
		fread(ConfBuf, buflen, 1, fi);
		ConfBuf[buflen] = CTLZ;     /*	can't use \0 */
		for (str = ConfBuf; *str; ++str) {
		    char *bup;
		    if (*str == '\n') {     /*  make separate strs */
			*str = 0;
					    /*	remove white space at end */
			for (bup = str - 1; bup >= ConfBuf && (*bup == ' ' || *bup == 9); --bup)
			    *bup = 0;
		    }
		}
	    } else {
		ConfBuf = NULL;
	    }
	}
    }
    if (ConfBuf == NULL)
	return(NULL);
    /*
     *	Search ConfBuf for Field<space/tab>
     */

    for (str = ConfBuf; *str != CTLZ; str += strlen(str) + 1) {
	if (*str == 0)
	    continue;
	if (strncmp(str, field, flen) == 0 && (str[flen] == ' ' || str[flen] == '\t')) {
	    str += flen;
	    while (*str == ' ' || *str == 9)
		++str;
	    return(str);
	}
    }
    return(NULL);
}

char *
GetConfig(field, def)
char *field;
char *def;
{
    char *result = FindConfig(field);

    if (result == NULL)
	result = def;
    return(result);
}

