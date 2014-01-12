
/*
 *  GETENV.C
 *
 *  Lattice's screws up.
 *
 *  (C) Copyright 1989-1990 by Matthew Dillon,  All Rights Reserved.
 */

#include <stdio.h>

char *
gettmpenv(id)
char *id;
{
    static char *buf;
    static char *res = NULL;
    long fh;
    long len;

    buf = (char *)malloc(strlen(id) + 8);
    sprintf(buf, "ENV:%s", id);
    fh = Open(buf, 1005);
    free(buf);
    if (fh) {
	Seek(fh, 0L, 1);
	len = Seek(fh, 0L, -1);
	if (len < 0) {
	    Close(fh);
	    return(NULL);
	}
	if (res)
	    free(res);
	res = (char *)malloc(len + 1);
	Read(fh, res, len);
	Close(fh);
	res[len] = 0;
	return(res);
    }
    return(NULL);
}

char *
getenv(id)
char *id;
{
    char *res = gettmpenv(id);
    char *perm = NULL;

    if (res) {
	perm = (char *)malloc(strlen(res) + 1);
	strcpy(perm, res);
    }
    return(perm);
}

