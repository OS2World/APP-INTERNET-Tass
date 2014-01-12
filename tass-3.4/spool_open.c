

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"tass.h"


/* Hopefully one of these is right for you. */

#ifdef BSD
#	include <sys/dir.h>
#	define		DIR_BUF		struct direct
#	define		D_LENGTH	d_namlen
#endif
#ifdef M_XENIX
#	include <sys/ndir.h>
#	define		DIR_BUF		struct direct
#	define		D_LENGTH	d_namlen
#endif
#ifndef DIR_BUF
#	include	<dirent.h>
#	define		DIR_BUF		struct dirent
#ifdef	OS2
#ifdef	__EMX__
#	define		D_LENGTH	d_reclen
#else
#	define		D_LENGTH	d_namlen
#endif
#else
#	define		D_LENGTH	d_reclen
#endif
#endif



char *
is_remote() {

	return "";
}

nntp_startup() {
}

nntp_finish() {
}


FILE *open_active_fp() {
	FILE *fp;

	fp = fopen(active_file, "r");
	if (fp == NULL) {
		fprintf(stderr, "can't open %s: ", active_file);
		perror("");
		exit(1);
	}

	return fp;
}



FILE *
open_art_fp(group_path, art)
char *group_path;
long art;
{
	char buf[LEN];
	struct stat sb;
	extern long note_size;

	sprintf(buf, "%s/%s/%ld", SPOOLDIR, group_path, art);

	if (stat(buf, &sb) < 0)
		note_size = 0;
	else
		note_size = sb.st_size;

	return fopen(buf, "r");
}


open_header_fd(group_path, art)
char *group_path;
long art;
{
	char buf[LEN];

	sprintf(buf, "%s/%s/%ld", SPOOLDIR, group_path, art);
	return open(buf, 0);
}



/*
 *  Longword comparison routine for the qsort()
 */

base_comp(a, b)
long *a;
long *b;
{

	if (*a < *b)
		return -1;
	if (*a > *b)
		return 1;
	return 0;
}


/*
 *  Read the article numbers existing in a group's spool directory
 *  into base[] and sort them.  top_base is one past top.
 */

setup_base(group, group_path)
char *group;
char *group_path;
{
	DIR *d;
	DIR_BUF *e;
	long art;
	char buf[LEN];

	top_base = 0;

	sprintf(buf, "%s/%s", SPOOLDIR, group_path);

	if (access(buf, 4) != 0)
		return;

	d = opendir(buf);

	if (d != NULL) {
		while ((e = readdir(d)) != NULL) {
			art = my_atol(e->d_name, /*e->D_LENGTH*/strlen (e->d_name));
			if (art >= 0) {
				if (top_base >= max_art)
					expand_art();
				base[top_base++] = art;
			}
		}
		closedir(d);
		qsort(base, top_base, sizeof(long), base_comp);
	}
}

