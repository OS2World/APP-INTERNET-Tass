

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"nntp.h"
#include	"tass.h"


char *
is_remote() {

	return " (remote)";
}


nntp_startup() {
	char *server_name;
	int ret;
	extern char *getenv();

	server_name = getserverbyfile(NNTP_SERVER);
	if (server_name == NULL) {
		fprintf(stderr, "Can't get nntp server name\n");
		fprintf(stderr, "Either put the name in the file %s, or put\n",
							NNTP_SERVER);
		fprintf(stderr, "it in the environment variable NNTPSERVER\n");
		exit(1);
	}

	ret = server_init(server_name);

	switch (ret) {
	case OK_CANPOST:
	case OK_NOPOST:
		break;

	case -1:
		fprintf(stderr, "failed to connect to server\n");
		exit(1);

	default:
		fprintf(stderr, "rejected by server, nntp error %d\n", ret);
		exit(1);
	}
}


nntp_finish() {
	close_server();
}


/*
 *  get_respcode
 *  get a response code from the server and return it to the caller
 */

int get_respcode() {
	char line[NNTP_STRLEN];

	if (get_server(line, NNTP_STRLEN) == -1) {
		fprintf(stderr, "connection to server broken\n");
		tass_done(1);
	}

	return atoi(line);
}



stuff_nntp(fnam)
char *fnam;
{
	FILE *fp;
	char line[NNTP_STRLEN];
	extern char *mktemp();
	struct stat sb;
	extern long note_size;
#ifdef OS2
	char * tmp;
	tmp = getenv("TMP");
	strcpy (fnam, tmp);
	strcat (fnam, "/");
	strcat (fnam, "tsXXXXXX");
#else
	strcpy(fnam, "/tmp/tass_nntpXXXXXX");
#endif
	mktemp(fnam);

	fp = fopen(fnam, "w");
	if (fp == NULL) {
		fprintf(stderr, "stuff_nntp: can't open %s: ", fnam);
		perror("");
		return FALSE;
	}

	while (1) {
		if (get_server(line, NNTP_STRLEN) == -1) {
			fprintf(stderr, "connection to server broken\n");
			tass_done(1);
		}
		if (strcmp(line, ".") == 0)
			break;			/* end of text */
		strcat(line, "\n");
		if (line[0] == '.')		/* reduce leading .'s */
			fputs(&line[1], fp);
		else
			fputs(line, fp);
	}
	fclose(fp);

	if (stat(fnam, &sb) < 0)
		note_size = 0;
	else
		note_size = sb.st_size;

	return TRUE;
}


FILE *
nntp_to_fp() {
	char fnam[LEN];
	FILE *fp;

	if (!stuff_nntp(fnam))
		return NULL;

	fp = fopen(fnam, "r");
	if (fp == NULL) {
		fprintf(stderr, "nntp_to_fp: can't reopen %s: ", fnam);
		perror("");
		return NULL;
	}
	unlink(fnam);
	return fp;
}


nntp_to_fd() {
	char fnam[LEN];
	int fd;

	if (!stuff_nntp(fnam))
		return NULL;

	fd = open(fnam, 0);
	if (fd == NULL) {
		fprintf(stderr, "nntp_to_fd: can't reopen %s: ", fnam);
		perror("");
		return -1;
	}
	unlink(fnam);
	return fd;
}



FILE *
open_active_fp() {

	put_server("list");
	if (get_respcode() != OK_GROUPS)
		return NULL;

	return nntp_to_fp();
}


FILE *
open_art_fp(group_path, art)
char *group_path;
long art;
{
	char buf[LEN];

	sprintf(buf, "article %ld", art);

	put_server(buf);
	if (get_respcode() != OK_ARTICLE)
		return NULL;

	return nntp_to_fp();
}


open_header_fd(group_path, art)
char *group_path;
long art;
{
	char buf[LEN];

	sprintf(buf, "head %ld", art);
	put_server(buf);
	if (get_respcode() != OK_HEAD)
		return -1;

	return nntp_to_fd();
}


nntp_stat(artnum)
long artnum;
{
	char line[LEN];

	sprintf(line, "stat %ld", artnum);
	put_server(line);
	if (get_respcode() != OK_NOTEXT)
		return FALSE;

	return TRUE;
}


setup_base(group, group_path)
char *group;
char *group_path;
{
	char buf[LEN];
	char line[NNTP_STRLEN];
	long start, last, dummy, count;

	top_base = 0;

	sprintf(buf, "group %s", group);
	put_server(buf);

	if (get_server(line, NNTP_STRLEN) == -1) {
		fprintf(stderr, "connection to server broken\n");
		tass_done(1);
	}

	if (atoi(line) != OK_GROUP)
		return;

	sscanf(line,"%ld %ld %ld %ld", &dummy, &count, &start, &last);
	if (last - count > start)
		start = last - count;

	while (start < last && !nntp_stat(start))
		start++;

	while (start <= last) {
		if (top_base >= max_art)
			expand_art();
		base[top_base++] = start++;
	}
}


