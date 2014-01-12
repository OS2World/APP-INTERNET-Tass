
#include	<stdio.h>
#include	"tass.h"


extern char note_h_followup[];
extern char note_h_subj[];
extern char note_h_distrib[];
extern char note_h_messageid[];
extern char note_h_from[];
extern char note_h_newsgroups[];
extern char note_h_reply[];

extern long	note_mark[];
extern FILE	*note_fp;

extern char	note_from_addr[];

#ifdef OS2
extern INFO my_stuff;
#include	<time.h>
#endif

void
log_post (char * nam) {
	FILE *	log, * tmp;
    	char   	buf[256];
    	char   	timestr[64];
    	struct tm *gmt, *tmnow;
    	time_t 	t;

        /* log it */
        strcpy(buf, SPOOLDIR);
        strcat(buf, "/post.log");
        if ((log = fopen(buf, "at")) != NULL) {
            fprintf(log, "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01"
                         "\x01\x01\x01\x01\x01\x01\x01\x01\x01\x01\n");
            time(&t);
            gmt = gmtime(&t);
	    tmnow = localtime(&t);
	    strftime(timestr, sizeof(timestr), "%a %b %d %H:%M:%S %z %Y", gmt);
	    fprintf(log, "From %s!%s %s\n", my_stuff.my_site, my_stuff.user, 
		    timestr);

	    tmp = fopen (nam, "rt");
            while (fgets(buf, 255, tmp) != NULL)  {
               fputs(buf, log);
            }
            fclose(tmp);

            fclose(log);
        }
}


/*
 *  Post an original article (not a followup)
 */

post_base(group)
char *group;
{
	FILE *fp;
	char nam[LEN];
	char ch;
	char subj[LEN+1];
	char buf[LEN];

	if (!parse_string("Subject: ", subj))
		return;
	if (subj[0] == '\0')
		return;

	setuid(real_uid);
	setgid(real_gid);

	sprintf(nam, "%s/.article", homedir);
	if ((fp = fopen(nam, "w")) == NULL) {
		fprintf(stderr, "can't open %s: ", nam);
		perror("");
		setuid(tass_uid);
		setgid(tass_gid);
		return(FALSE);
	}
#ifndef	OS2
	chmod(nam, 0600);
#endif

	if (*my_from)
		fprintf(fp, "From: %s\n", my_from);

	fprintf(fp, "Subject: %s\n", subj);
	fprintf(fp, "Newsgroups: %s\n", group);
	fprintf(fp, "Distribution: \n");
	if (*my_org)
		fprintf(fp, "Organization: %s\n", my_org);
	fprintf(fp, "\n");

	add_signature(fp, TRUE);
	fclose(fp);

	ch = 'e';
	while (1) {
		switch (ch) {
		case 'e':
			invoke_editor(nam);
			break;

		case 'a':
			setuid(tass_uid);
			setgid(tass_gid);
			return FALSE;

		case 'p':
			printf("\nPosting...  ");
			fflush(stdout);
#ifdef	OS2
			log_post (nam);
			sprintf(buf, "inews -h < %s", nam);
#else
			sprintf(buf, "%s/inews -h < %s", libdir, nam);
#endif
			if (invoke_cmd(buf)) {
				printf("article posted\n");
				fflush(stdout);
				goto post_base_done;
			} else {
				printf("article rejected\n");
				fflush(stdout);
				break;
			}
		}

		do {
			MoveCursor(LINES, 0);
			fputs("abort, edit, post: ", stdout);
			fflush(stdout);
			ch = ReadCh();
		} while (ch != 'a' && ch != 'e' && ch != 'p');
	}

post_base_done:
	setuid(tass_uid);
	setgid(tass_gid);

	continue_prompt();

	return(TRUE);
}

static int do_cc = 0;

/*
 *  Post and cc a followup
 */

post_cc_response(group, copy_text)
char *group;
int copy_text;
{
    do_cc = 1;
    post_response(group, copy_text);
    do_cc = 0;
}

post_response(group, copy_text)
char *group;
int copy_text;
{
	FILE *fp;
	char nam[LEN];
	char ch;
	char buf[LEN];
	char mail_to[LEN+1];

	if (*note_h_followup && strcmp(note_h_followup, "poster") == 0) {
		clear_message();
		MoveCursor(LINES,0);
		printf("Note: Responses have been directed to the poster");
		if (!prompt_yn("Post anyway? (y/n): "))
			return FALSE;
		*note_h_followup = '\0';
	} else if (*note_h_followup && strcmp(note_h_followup, group) != 0) {
	    clear_message();
	    MoveCursor(LINES,0);
	    printf("Note:  Responses have been directed to %s\r\n\r\n",
							note_h_followup);
	    if (!prompt_yn("Continue? (y/n): "))
		return FALSE;
	}

	setuid(real_uid);
	setgid(real_gid);

	sprintf(nam, "%s/.article", homedir);
	if ((fp = fopen(nam, "w")) == NULL) {
		fprintf(stderr, "can't open %s: ", nam);
		perror("");
		setuid(tass_uid);
		setgid(tass_gid);
		return FALSE;
	}
#ifndef	OS2
	chmod(nam, 0600);
#endif
	
	if (*my_from)
		fprintf(fp, "From: %s\n", my_from);
        if (do_cc)
                fprintf(fp, "To: %s\n", note_h_from);
	fprintf(fp, "Subject: Re: %s\n", eat_re(note_h_subj));

	if (*note_h_followup && strcmp(note_h_followup, "poster") != 0)
		fprintf(fp, "Newsgroups: %s\n", note_h_followup);
	else
	        fprintf(fp, "Newsgroups: %s\n", note_h_newsgroups);
	if (*my_org)
		fprintf(fp, "Organization: %s\n", my_org);

	if (note_h_distrib != '\0')
		fprintf(fp, "Distribution: %s\n", note_h_distrib);

	fprintf(fp, "References: %s\n", note_h_messageid);
	fprintf(fp, "\n");

	if (copy_text) {
		fprintf(fp, "%s writes:\n", note_h_from);

		fseek(note_fp, note_mark[0], 0);
		copy_fp(note_fp, fp, "> ");
	}

	add_signature(fp, TRUE);
	fclose(fp);

	ch = 'e';
	while (1) {
		switch (ch) {
		case 'e':
			invoke_editor(nam);
			break;

		case 'a':
			setuid(tass_uid);
			setgid(tass_gid);
			return FALSE;

		case 'p':
			printf("Posting...  ");
			fflush(stdout);
#ifdef	OS2
			log_post (nam);
			sprintf(buf, "inews -h < %s", nam);
#else
			sprintf(buf, "%s/inews -h < %s", libdir, nam);
#endif
			if (invoke_cmd(buf)) {
				printf("article posted\r\n");
				fflush(stdout);
			} else {
				printf("article rejected\r\n");
				fflush(stdout);
				break;
			}
			if (do_cc) {
/*
 *  Open letter an get the To: line in case they changed it with
 *  the editor
 */

				find_new_to(nam, mail_to);
				printf("\r\nMailing to %s...", mail_to);
				fflush(stdout);
#ifdef OS2
				sprintf(buf, "%s -t -f %s", MAILER, nam);
#else
				sprintf(buf, "%s %s < %s", MAILER,
							mail_to, nam);
#endif
				if (invoke_cmd(buf)) {
					printf("Message sent\r\n");
					fflush(stdout);
				} else {
					printf("Command failed: %s\r\n", buf);
					fflush(stdout);
					break;
				}
			}
       			goto post_response_done;
		}

		do {
			MoveCursor(LINES, 0);
			fputs("abort, edit, post: ", stdout);
			fflush(stdout);
			ch = ReadCh();
		} while (ch != 'a' && ch != 'e' && ch != 'p');
	}

post_response_done:
	setuid(tass_uid);
	setgid(tass_gid);

	continue_prompt();

	return TRUE;
}


mail_to_someone(bounce)
int bounce;
{
	char nam[LEN];
	FILE *fp;
	char ch;
	char buf[LEN];
	char mail_to[LEN+1];
	char subj[LEN+1];

	if (bounce)
	{
		if (!parse_string("Bounce article to: ", mail_to))
			return;
	} else {
		if (!parse_string("Mail article to: ", mail_to))
			return;
	}

	if (mail_to[0] == '\0')
		return;

	setuid(real_uid);
	setgid(real_gid);

	sprintf(nam, "%s/.letter", homedir);
	if ((fp = fopen(nam, "w")) == NULL) {
		fprintf(stderr, "can't open %s: ", nam);
		perror("");
		setuid(tass_uid);
		setgid(tass_gid);
		return(FALSE);
	}
#ifndef	OS2
	chmod(nam, 0600);
#endif

	if (bounce)
	{
		fprintf(fp, "To: %s\n", mail_to);
		fseek(note_fp, 0L, 0);
		copy_fp(note_fp, fp, "");
	}
	else
	{
		if (*my_from)
			fprintf(fp, "From: %s\n", my_from);
		fprintf(fp, "To: %s\n", mail_to);
		fprintf(fp, "Subject: %s\n", note_h_subj);
		if (*note_h_followup)
			fprintf(fp, "Newsgroups: %s\n\n", note_h_followup);
		else
			fprintf(fp, "Newsgroups: %s\n", note_h_newsgroups);
		if (*my_org)
			fprintf(fp, "Organization: %s\n", my_org);
		fputs("\n", fp);

		fseek(note_fp, 0L, 0);
		copy_fp(note_fp, fp, "");

		add_signature(fp, TRUE);
	}

	fclose(fp);

	while (1) {
		do {
			MoveCursor(LINES, 0);
			fputs("abort, edit, send: ", stdout);
			fflush(stdout);
			ch = ReadCh();
		} while (ch != 'a' && ch != 'e' && ch != 's');

		switch (ch) {
		case 'e':
			invoke_editor(nam);
			break;

		case 'a':
			setuid(tass_uid);
			setgid(tass_gid);
			return FALSE;

		case 's':
/*
 *  Open letter an get the To: line in case they changed it with
 *  the editor
 */

			find_new_to(nam, mail_to);
			printf("\r\nMailing to %s...", mail_to);
			fflush(stdout);
#ifdef OS2
			sprintf(buf, "%s -t -f %s", MAILER, nam);
#else
			sprintf(buf, "%s %s < %s", MAILER,
							mail_to, nam);
#endif
			if (invoke_cmd(buf)) {
				printf("Message sent\r\n");
				fflush(stdout);
				goto mail_to_someone_done;
			} else {
				printf("Command failed: %s\r\n", buf);
				fflush(stdout);
				break;
			}
		}
	}

mail_to_someone_done:
	setuid(tass_uid);
	setgid(tass_gid);

	page_cont();

	return TRUE;
}


mail_to_author(copy_text)
int copy_text;
{
	char nam[LEN];
	FILE *fp;
	char ch;
	char buf[LEN];
	char mail_to[LEN+1];
	char *who_to;

	setuid(real_uid);
	setgid(real_gid);

	who_to = note_h_from;
	printf("\r\nMailing to %s...\r\n\r\n", who_to);

	sprintf(nam, "%s/.letter", homedir);
	if ((fp = fopen(nam, "w")) == NULL) {
		fprintf(stderr, "can't open %s: ", nam);
		perror("");
		setuid(tass_uid);
		setgid(tass_gid);
		return(FALSE);
	}
#ifndef	OS2
	chmod(nam, 0600);
#endif

	if (*my_from)
		fprintf(fp, "From: %s\n", my_from);
	fprintf(fp, "To: %s\n", who_to);
	fprintf(fp, "Subject: Re: %s\n", eat_re(note_h_subj) );
	fprintf(fp, "Newsgroups: %s\n", note_h_newsgroups);
	if (*my_org)
		fprintf(fp, "Organization: %s\n", my_org);
	fputs("\n", fp);

	if (copy_text) {		/* if "copy_text" */
		fprintf(fp, "In article %s you write:\n", note_h_messageid);

		fseek(note_fp, note_mark[0], 0);
		copy_fp(note_fp, fp, "> ");
	}

	add_signature(fp, TRUE);
	fclose(fp);

	ch = 'e';
	while (1) {
		switch (ch) {
		case 'e':
			invoke_editor(nam);
			break;

		case 'a':
			setuid(tass_uid);
			setgid(tass_gid);
			return FALSE;

		case 's':
			strcpy(mail_to, note_from_addr);
			find_new_to(nam, mail_to);
			printf("\r\nMailing to %s...  ", mail_to);
			fflush(stdout);
#ifdef OS2
			sprintf(buf, "%s -t -f %s", MAILER, nam);
#else
			sprintf(buf, "%s %s < %s", MAILER, mail_to, nam);
#endif
			if (invoke_cmd(buf)) {
				printf("Message sent\r\n");
				fflush(stdout);
				goto mail_to_author_done;
			} else {
				printf("Command failed: %s\r\n", buf);
				fflush(stdout);
				break;
			}
		}

		do {
			MoveCursor(LINES, 0);
			fputs("abort, edit, send: ", stdout);
			fflush(stdout);
			ch = ReadCh();
		} while (ch != 'a' && ch != 'e' && ch != 's');
	}

mail_to_author_done:
	setuid(tass_uid);
	setgid(tass_gid);

	page_cont();

	return TRUE;
}


post_cancel(group)
char *group;
{
	FILE *fp;
	char nam[LEN];
	char ch;
	char buf[LEN];

	if (!prompt_yn("Cancel this article? (y/n): "))
		return FALSE;

	setuid(real_uid);
	setgid(real_gid);

	sprintf(nam, "%s/.article", homedir);
	if ((fp = fopen(nam, "w")) == NULL) {
		fprintf(stderr, "can't open %s: ", nam);
		perror("");
		setuid(tass_uid);
		setgid(tass_gid);
		return FALSE;
	}
#ifdef	OS2
	chmod(nam, 0600);
#endif

	if (*my_from)
		fprintf(fp, "From: %s\n", my_from);
	fprintf(fp, "Subject: Re: %s\n", eat_re(note_h_subj));
	fprintf(fp, "Newsgroups: %s\n", note_h_newsgroups);

	if (*my_org)
		fprintf(fp, "Organization: %s\n", my_org);

	if (note_h_distrib != '\0')
		fprintf(fp, "Distribution: %s\n", note_h_distrib);

	fprintf(fp, "Control: cancel %s\n", note_h_messageid);
	fprintf(fp, "\n");
	fprintf(fp, "cancel %s\n", note_h_messageid);

	add_signature(fp, TRUE);
	fclose(fp);

	while (1) {
		do {
			MoveCursor(LINES, 0);
			fputs("abort, edit, post: ", stdout);
			fflush(stdout);
			ch = ReadCh();
		} while (ch != 'a' && ch != 'e' && ch != 'p');

		switch (ch) {
		case 'e':
			invoke_editor(nam);
			break;

		case 'a':
			setuid(tass_uid);
			setgid(tass_gid);
			return FALSE;

		case 'p':
			printf("Posting...  ");
			fflush(stdout);
#ifdef	OS2
			log_post (nam);
			sprintf(buf, "inews -h < %s", nam);
#else
			sprintf(buf, "%s/inews -h < %s", libdir, nam);
#endif
			if (invoke_cmd(buf)) {
				printf("article posted\r\n");
				fflush(stdout);
				goto post_cancel_done;
			} else {
				printf("article rejected\r\n");
				fflush(stdout);
				break;
			}
		}
	}

post_cancel_done:
	setuid(tass_uid);
	setgid(tass_gid);

	page_cont();

	return TRUE;
}

