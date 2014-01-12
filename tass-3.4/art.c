

#include	<stdio.h>
#include	<ctype.h>
#include	<signal.h>
#ifdef	OS2
#include	<time.h>
#endif
#include	"tass.h"


#define	BIGLEN	4096


char index_file[LEN+1];
char *glob_art_group;
extern char *hash_str();


#ifdef SIGTSTP
void
art_susp(i)
int i;
{

	Raw(FALSE);
	putchar('\n');
	signal(SIGTSTP, SIG_DFL);
#ifdef BSD
        sigsetmask(sigblock(0) & ~(1 << (SIGTSTP - 1)));
#endif
	kill(0, SIGTSTP);

	signal(SIGTSTP, art_susp);
	Raw(TRUE);

	mail_setup();
	ClearScreen();
	MoveCursor(LINES, 0);
	printf("Group %s...    ", glob_art_group);
	fflush(stdout);
}
#endif


/*
 *  Convert a string to a long, only look at first n characters
 */

my_atol(s, n)
char *s;
int n;
{
	long ret = 0;

	while (*s && n--) {
		if (*s >= '0' && *s <= '9')
			ret = ret * 10 + (*s - '0');
		else
			return -1;
		s++;
	}

	return ret;
}


/*
 *  Construct the pointers to the basenotes of each thread
 *  arts[] contains every article in the group.  inthread is
 *  set on each article that is after the first article in the
 *  thread.  Articles which have been expired have their thread
 *  set to -2.
 */

find_base() {
	int i;

	top_base = 0;

	for (i = 0; i < top; i++)
		if (!arts[i].inthread && arts[i].thread != -2)
		{
			if (top_base >= max_art)
				expand_art();
			base[top_base++] = i;
		}
}


/* 
 *  Count the number of non-expired articles in arts[]
 */

num_arts() {
	int sum = 0;

	int i;

	for (i = 0; i < top; i++)
		if (arts[i].thread != -2)
			sum++;

	return sum;
}


/*
 *  Do we have an entry for article art?
 */

valid_artnum(art)
long art;
{
	int i;

	for (i = 0; i < top; i++)
		if (arts[i].artnum == art)
			return i;

	return -1;
}


/*
 *  Return TRUE if arts[] contains any expired articles
 *  (articles we have an entry for which don't have a corresponding
 *   article file in the spool directory)
 */

purge_needed() {
	int i;

	for (i = 0; i < top; i++)
		if (arts[i].thread == -2)
			return TRUE;

	return FALSE;
}


/*
 *  Main group indexing routine.  Group should be the name of the
 *  newsgroup, i.e. "comp.unix.amiga".  group_path should be the
 *  same but with the .'s turned into /'s: "comp/unix/amiga"
 *
 *  Will read any existing index, create or incrementally update
 *  the index by looking at the articles in the spool directory,
 *  and attempt to write a new index if necessary.
 */

index_group(group, group_path)
char *group;
char *group_path;
{
	int modified;

	glob_art_group = group;

#ifdef SIGTSTP
	signal(SIGTSTP, art_susp);
#endif

	if (!update) {
		clear_message();
		MoveCursor(LINES, 0);
		printf("Group %s...    ", group);
		fflush(stdout);
	}

	hash_reclaim();
	if (local_index)
		find_local_index(group);
	else
		sprintf(index_file, "%s/%s/.tindx", SPOOLDIR, group_path);

	load_index();
	modified = read_group(group, group_path);
	make_threads();
	if (modified || purge_needed()) {
		if (local_index) {	/* writing index in home directory */
			setuid(real_uid);	/* so become them */
			setgid(real_gid);
		}
		dump_index(group);
		if (local_index) {
			setuid(tass_uid);
			setgid(tass_gid);
		}
	}
	find_base();

	if (modified && !update)
		clear_message();
}


/*
 *  Index a group.  Assumes any existing index has already been
 *  loaded.
 */

read_group(group, group_path)
char *group;
char *group_path;
{
	int fd;
	long art;
	int count;
	int modified = FALSE;
	int respnum;
	int i;
#ifdef XTRA
	int total = 0;
#endif

	setup_base(group, group_path);	  /* load article numbers into base[] */
	count = 0;

#ifdef XTRA
	for (i = 0; i < top_base; i++)
	{
		if (valid_artnum(base[i]) < 0)
			total++;
	}
#endif

	for (i = 0; i < top_base; i++) {	/* for each article # */
		art = base[i];

/*
 *  Do we already have this article in our index?  Change thread from
 *  -2 to -1 if so and skip the header eating.
 */

		if ((respnum = valid_artnum(art)) >= 0) {
			arts[respnum].thread = -1;
			arts[respnum].unread = 1;
			continue;
		}

		if (!modified)
			modified = TRUE;   /* we've modified the index */
					   /* it will need to be re-written */

		fd = open_header_fd(group_path, art);
		if (fd < 0)
			continue;

/*
 *  Add article to arts[]
 */

		if (top >= max_art)
			expand_art();

		arts[top].artnum = art;
		arts[top].thread = -1;
		arts[top].inthread = FALSE;
		arts[top].unread = 1;

		if (!parse_headers(fd, &arts[top])) {
			close(fd);
			continue;
		}
		top++;
		close(fd);

		count++;
		if (count % 10 == 0 && !update) {
			if (count == 10)
				printf("\b\b\b\b%4d/%-5d\b\b\b\b\b\b",
							count, total);
			else
				printf("\b\b\b\b%4d", count);
			fflush(stdout);
		}
	}

	return modified;
}


/*
 *  Go through the articles in arts[] and use .thread to snake threads
 *  through them.  Use the subject line to construct threads.  The
 *  first article in a thread should have .inthread set to FALSE, the
 *  rest TRUE.  Only do unexpired articles we haven't visited yet
 *  (arts[].thread == -1).
 */

make_threads()
{
	int i;
	int j;

	for (i = 0; i < top; i++)
	{
		if (arts[i].thread == -1)
		    for (j = i+1; j < top; j++)
			if (arts[j].thread == -1
			&&  arts[i].subject == arts[j].subject)
			{
				arts[i].thread = j;
				arts[j].inthread = TRUE;
				break;
			}
	}
}


/*
 *  Return a pointer into s eliminating any leading Re:'s.  Example:
 *
 *	  Re: Reorganization of misc.jobs
 *	  ^   ^
 */

char *
eat_re(s)
char *s;
{
	
	while (*s == ' ')
		s++;

	while (*s == 'r' || *s == 'R') {
		if ((*(s+1) == 'e' || *(s+1) == 'E')) {
			if (*(s+2) == ':')
				s += 3;
			else if (*(s+2) == '^' && isdigit(*(s+3)) && *(s+4) == ':')
				s += 5;			/* hurray nn */
			else
				break;
		} else
			break;
		while (*s == ' ')
			s++;
	}

	return s;
}


parse_headers(fd, h)
int fd;
struct header *h;
{
	char buf[4096];
	char *p, *q;
	char flag;
	int n;
	char buf2[4096];
	char *s;

	n = read(fd, buf, 4096);
	if (n <= 0)
		return FALSE;

	buf[n - 1] = '\0';

	h->subject = "";
	h->from = "";

	p = buf;
	while (1) {
		for (q = p; *p && *p != '\n'; p++)
			if (((*p) & 0x7F) < 32)
				*p = ' ';
		flag = *p;
		*p++ = '\0';

		if (strncmp(q, "From: ", 6) == 0) {
			strncpy(buf2, &q[6], LEN-1);
			buf2[LEN-1] = '\0';
			h->from = hash_str(buf2);
		} else if (strncmp(q, "Subject: ", 9) == 0) {
			strncpy(buf2, &q[9], LEN-1);
			buf2[LEN-1] = '\0';
			s = eat_re(buf2);
			h->subject = hash_str(eat_re(s));
		}

		if (!flag || *p == '\n')
			break;
	}

	return TRUE;
}


/* 
 *  Write out a .tindx file.  Write the group name first so if
 *  local indexing is done we can disambiguate between group name
 *  hash collisions by looking at the index file.
 */

dump_index(group)
char *group;
{
	int i;
	char buf[LEN];
	char nam[LEN];
	FILE *fp;
	int *iptr;
	int realnum;

#ifdef	OS2
	srand (time (NULL));
	sprintf(nam, "%s.%d", index_file, rand());
#else
	sprintf(nam, "%s.%d", index_file, getpid());
#endif
	fp = fopen(nam, "w");

	if (fp == NULL)
		return;

	fprintf(fp, "%s\n", group);
	fprintf(fp, "%d\n", num_arts());

	realnum = 0;
	for (i = 0; i < top; i++)
		if (arts[i].thread != -2) {
			fprintf(fp, "%ld\n", arts[i].artnum);

			iptr = (int *) arts[i].subject;
			iptr--;

			if (arts[i].subject[0] == '\0')
				fprintf(fp, " %s\n", arts[i].subject);
			else if (*iptr < 0) {
				fprintf(fp, " %s\n", arts[i].subject);
				*iptr = realnum;
			} else
				fprintf(fp, "%%%d\n", *iptr);

			iptr = (int *) arts[i].from;
			iptr--;

			if (arts[i].from[0] == '\0')
				fprintf(fp, " %s\n", arts[i].from);
			else if (*iptr < 0) {
				fprintf(fp, " %s\n", arts[i].from);
				*iptr = realnum;
			} else
				fprintf(fp, "%%%d\n", *iptr);

			realnum++;
		}

	fclose(fp);
#ifndef	OS2
	chmod(nam, 0644);
#endif
	unlink(index_file);
	link(nam, index_file);
	unlink(nam);
}


/*
 *  strncpy that stops at a newline and null terminates
 */

my_strncpy(p, q, n)
char *p;
char *q;
int n;
{

	while (n--) {
		if (!*q || *q == '\n')
			break;
		*p++ = *q++;
	}
	*p = '\0';
}


/*
 *  Read in a .tindx file.
 */

load_index()
{
	int i;
	long j;
	char buf[BIGLEN];
	FILE *fp;
	int first = TRUE;
	char *p;
	int n;
	char *err;

	top = 0;

	fp = fopen(index_file, "r");
	if (fp == NULL)
		return;

	if (fgets(buf, BIGLEN, fp) == NULL
	||  fgets(buf, BIGLEN, fp) == NULL) {
		err = "one";
		goto corrupt_index;
	}

	i = atol(buf);
	while (top < i) {
		if (top >= max_art)
			expand_art();

		arts[top].thread = -2;
		arts[top].inthread = FALSE;

		if (fgets(buf, BIGLEN, fp) == NULL) {
			err = "two";
			goto corrupt_index;
		}
		arts[top].artnum = atol(buf);

		if (fgets(buf, BIGLEN, fp) == NULL) {
			err = "three";
			goto corrupt_index;
		}

		if (buf[0] == '%') {
			n = atoi(&buf[1]);
			if (n >= top || n < 0) {
				err = "eight";
				goto corrupt_index;
			}
			arts[top].subject = arts[n].subject;
		} else if (buf[0] == ' ') {
			for (p = &buf[1]; *p && *p != '\n'; p++) ;
			*p = '\0';
			buf[BIGLEN-1] = '\0';
			arts[top].subject = hash_str(&buf[1]);
		} else {
			err = "six";
			goto corrupt_index;
		}
				
		if (fgets(buf, BIGLEN, fp) == NULL) {
			err = "four";
			goto corrupt_index;
		}

		if (buf[0] == '%') {
			n = atoi(&buf[1]);
			if (n >= top || n < 0) {
				err = "nine";
				goto corrupt_index;
			}
			arts[top].from = arts[n].from;
		} else if (buf[0] == ' ') {
			for (p = &buf[1]; *p && *p != '\n'; p++) ;
			*p = '\0';
			buf[BIGLEN-1] = '\0';
			arts[top].from = hash_str(&buf[1]);
		} else {
			err = "seven";
			goto corrupt_index;
		}

		top++;
	}

	fclose(fp);
	return;

corrupt_index:
	fprintf(stderr, "\r\n%s: index file %s corrupt, top=%d\r\n",
						err, index_file, top);
	unlink(index_file);
	top = 0;
}


/*
 *  Look in the local $HOME/.tindx (or wherever) directory for the
 *  index file for the given group.  Hashing the group name gets
 *  a number.  See if that #.1 file exists; if so, read first line.
 *  Group we want?  If no, try #.2.  Repeat until no such file or
 *  we find an existing file that matches our group.
 */

find_local_index(group)
char *group;
{
	unsigned long h;
	static char buf[LEN];
	int i;
	char *p;
	FILE *fp;

	h = hash_groupname(group);

	i = 1;
	while (1) {
		sprintf(index_file, "%s/%lu.%d", indexdir, h, i);
		fp = fopen(index_file, "r");
		if (fp == NULL)
			return;

		if (fgets(buf, LEN, fp) == NULL) {
			fclose(fp);
			return;
		}
		fclose(fp);

		for (p = buf; *p && *p != '\n'; p++) ;
		*p = '\0';

		if (strcmp(buf, group) == 0)
			return;

		i++;
	}
}


/*
 *  Run the index file updater only for the groups we've loaded.
 */

do_update() {
	int i;
	char group_path[LEN];
	char *p;

	for (i = 0; i < local_top; i++) {
		strcpy(group_path, active[my_group[i]].name);
		for (p = group_path; *p; p++)
			if (*p == '.')
				*p = '/';

		index_group(active[my_group[i]].name, group_path);
	}
}

