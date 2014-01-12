

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"tass.h"



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


FILE *
open_archive_fp(group_path)
char *group_path;
{
	DIR *d;
	DIR_BUF *e;
	long art = 0;
	long i;
	char buf[LEN];
	FILE *fp;

	sprintf(buf, "%s/.news/%s", homedir, group_path);

	d = opendir(buf);

	if (d == NULL)
	{
		art = 1;
		mkdir_p(buf);
	}
	else
	{
		while ((e = readdir(d)) != NULL) {
			i = my_atol(e->d_name, /*e->D_LENGTH*/strlen (e->d_name));
			if (i > art)
				art = i;
		}
		closedir(d);
		art++;
	}

	sprintf(buf, "%s/.news/%s/%ld", homedir, group_path, art);
	fp = fopen(buf, "w");

	if (fp == NULL)
	{
		fprintf(stderr, "can't write %s: ", buf);
		perror("");
	}

	return fp;
}


archive_article(group_path)
char *group_path;
{
	FILE *fp;
	extern FILE *note_fp;
	extern long note_mark[];
	extern int note_page;

	setuid(real_uid);
	setgid(real_gid);

	fp = open_archive_fp(group_path);

	if (fp == NULL)
	{
		setuid(real_uid);
		setgid(real_gid);

		page_cont();
		return;
	}

	fseek(note_fp, 0L, 0);
	copy_fp(note_fp, fp, "");
	fclose(fp);
	fseek(note_fp, note_mark[note_page], 0);

	setuid(real_uid);
	setgid(real_gid);
	info_message("-- article archived --");
}


archive_thread(respnum, group_path)
int respnum;
char *group_path;
{
	FILE *fp;
	FILE *art;
	int i;
	int b;
	int count = 0;
	char *p;
	extern FILE *note_fp;
	extern long note_mark[];
	extern int note_page;

	b = which_base(respnum);

	setuid(real_uid);
	setgid(real_gid);

	MoveCursor(LINES, 0);
	fputs("Archiving...    ", stdout);
	fflush(stdout);

	note_save();

	for (i = base[b]; i >= 0; i = arts[i].thread)
	{
		fp = open_archive_fp(group_path);

		if (fp == NULL)
		{
			setuid(real_uid);
			setgid(real_gid);

			note_reopen(arts[respnum].artnum, group_path);
			fseek(note_fp, note_mark[note_page], 0);

			page_cont();
			return;
		}

		open_note(arts[i].artnum, group_path);

		fseek(note_fp, 0L, 0);
		copy_fp(note_fp, fp, "");

		note_cleanup();

		printf("\b\b\b\b%4d", ++count);
		fflush(stdout);

		fclose(fp);
	}

	setuid(real_uid);
	setgid(real_gid);

	info_message("-- thread archived --");

	note_reopen(arts[respnum].artnum, group_path);
	fseek(note_fp, note_mark[note_page], 0);
}

