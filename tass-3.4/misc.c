
#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	<signal.h>
#include	<pwd.h>
#ifdef	OS2
#include	<assert.h>
#endif
#include	<sys/types.h>
#include	<sys/stat.h>
#include	"tass.h"

#ifdef OS2
INFO my_stuff;
#endif

char active_file[LEN];
char homedir[LEN];
char userid[LEN];
char delgroups[LEN];
char newsrc[LEN];
char libdir[LEN];
char newnewsrc[LEN];
char indexdir[LEN];
char my_org[LEN];		/* organization */
char sig[LEN];
char signature[LEN];
char my_from[LEN];

#ifdef OS2
char confdir[256];
char spooldir[256];
char mailbox[256];
char *LIBDIR;
char *SPOOLDIR;

extern char * mailbox_name;
static char editor[200];
char mailer[200];
#endif

/*
 *  Which base note (an index into base[]) does a respnum
 *  (an index into arts[]) corresponsd to?
 *
 *  In other words, base[] points to an entry in arts[] which is
 *  the head of a thread, linked with arts[].thread.  For any q: arts[q],
 *  find i such that base[i]->arts[n]->arts[o]->...->arts[q]
 */

which_base(n)
int n;
{
	int i, j;

	for (i = 0; i < top_base; i++)
		for (j = base[i]; j >= 0; j = arts[j].thread)
			if (j == n)
				return i;

	fprintf(stderr, "which_base:  can't find base article\n");
	return 0;
}


/*
 *  Find how deep in a thread a response is.  Start counting at zero
 */

which_resp(n)
int n;
{
	int i, j;
	int num = 0;

	i = which_base(n);

	for (j = base[i]; j != -1; j = arts[j].thread)
		if (j == n)
			break;
		else
			num++;

	return num;
}


/*
 *  Given an index into base[], find the number of responses for
 *  that basenote
 */

nresp(n)
int n;
{
	int i;
	int oldi = -3;
	int sum = 0;

	assert(n < top_base);

	for (i = base[n]; i != -1; i = arts[i].thread) {
		assert(i != -2);
		assert(i != oldi);
		oldi = i;
		sum++;
	}

	return sum - 1;
}


/*
 *  Find the previous response.  Go to the last response in the previous
 *  thread if we go past the beginning of this thread.
 */

prev_response(n)
int n;
{
	int i, j;
	int prev;

	for (i = 0; i < top_base; i++)
		for (prev = -1, j = base[i]; j >= 0; j = arts[j].thread)
			if (j == n)
				goto prevbreak;
			else
				prev = j;

	fprintf(stderr, "prev_response:  can't find base article\n");
	return -1;

prevbreak:
	if (prev >= 0)
		return prev;
	else if (i <= 0)
		return -1;
	else {
		for (j = base[i-1]; arts[j].thread >= 0; j = arts[j].thread) ;
		return j;
	}
}


/*
 *  Find the next response.  Go to the next basenote if there
 *  are no more responses in this thread
 */

next_response(n)
int n;
{
	int i;

	if (arts[n].thread >= 0)
		return arts[n].thread;

	i = which_base(n) + 1;

	if (i >= top_base)
		return -1;

	return base[i];
}


/*
 *  Given a respnum (index into arts[]), find the respnum of the
 *  next basenote
 */

next_basenote(n)
int n;
{
	int i;

	i = which_base(n) + 1;
	if (i >= top_base)
		return -1;

	return base[i];
}



/*
 *  Find the next unread response in this group 
 */

next_unread(n)
int n;
{

	while (n >= 0) {
		if (arts[n].unread == 1)
			return n;
		n = next_response(n);
	}

	return -1;
}


/*
 *  Find the previous unread response in this thread
 */

prev_unread(n)
int n;
{

	while (n >= 0) {
		if (arts[n].unread == 1)
			return n;
		n = prev_response(n);
	}

	return -1;
}


asfail(file, line, cond)
char	*file;
int	line;
char	*cond;
{
	fprintf(stderr, "tass: assertion failure: %s (%d): %s\n",
							file, line, cond);
	exit(1);
}


/*
 * init_selfinfo
 *   Deterimines users home directory, userid, and a path
 *   for an rc file in the home directory
 */

init_selfinfo()
{
	struct passwd *myentry;
	extern struct passwd *getpwuid();
	struct stat sb;
	char nam[LEN];
	char *p;
	FILE *fp;

#ifdef OS2
	static char buf[256];
    	int  i;
    	char *fn, *v;
    	FILE *tmp;

    	my_stuff.replyto[0] = '\0';
    	editor[0] = '\0';
    	mailer[0] = '\0';

	/*--------------------- load UUPC rc files ---------------------------*/

    	/* read the system file first */
	for (i = 0; i < 2; i++) {
            /* choose the file to open */
            if (i == 0) {
            	fn = getenv("UUPCSYSRC");
            	if (fn == NULL)
       		    fprintf(stderr,
			"Enviroment variable UUPCSYSRC not defined\n");
            } else {
            	fn = getenv("UUPCUSRRC");
            	if (fn == NULL)
                    fprintf(stderr,
			 "Enviroment variable UUPCUSRRC not defined\n");
            }
	    if ((tmp = fopen(fn, "rt")) != NULL) {
		    while (fgets(buf, 255, tmp)) {
		        p = buf + strlen (buf) - 1;
		        if (*p == '\n')
			        *p = '\0';
		        if (p > buf)
		                if (*(p-1) == '\n')
			                *(p-1) = '\0';
        		if (strnicmp(buf, "confdir=", 8) == 0) {
                		strcpy(confdir, buf+8);
                		LIBDIR = confdir;
                	}
               		else if (strnicmp(buf, "newsdir=", 8) == 0) {
               		    SPOOLDIR = spooldir;
                	    strcpy(SPOOLDIR, buf+8);
               		}
	               	else if (strnicmp(buf, "mailbox=", 8) == 0) {
			    	mailbox_name = getenv("MAIL");
				if (mailbox_name == NULL) {
	               	    		mailbox_name = mailbox;
					strcpy(mailbox_name, buf+8);
				}
        	        }
                    	else if (strnicmp (buf, "Signature=", 10) == 0) {
                       	    strcpy (sig, buf+10);
                	}
		        else if (strnicmp (buf, "mailserv=", 9) == 0) {
                      	    strcpy (my_stuff.mail_server, buf+9);
                	}
                	else if (strnicmp (buf, "nodename=", 9) == 0) {
                       	    strcpy (my_stuff.my_site, buf+9);
		        }
                	else if (strnicmp (buf, "domain=", 7) == 0) {
                       	    strcpy (my_stuff.my_domain, buf+7);
                	}
                	else if (strnicmp(buf, "tempdir=", 8) == 0) {
                       	    strcpy(my_stuff.temp_name, buf+8);
                	}
                	else if (strnicmp(buf, "mailbox=", 8) == 0) {
                      	    strcpy(my_stuff.user, buf+8);
                	}
                	else if (strnicmp(buf, "Replyto=", 8) == 0) {
                      	    strcpy(my_stuff.replyto, buf+8);
                	}
                	else if (strnicmp(buf, "name=", 5) == 0) {
		            strcpy(my_stuff.my_name, buf+5);
		        }
                	else if (strnicmp(buf, "Organization=", 13) == 0) {
                       	    strcpy(my_org, buf+13);
                	}
                	else if (strnicmp (buf, "Editor=", 7) == 0) {
                      	    strcpy (editor, buf+7);
                	}
                	else if (strnicmp (buf, "Mailer=", 7) == 0) {
                      	    strcpy (mailer, buf+7);
                	}
                	else if (strnicmp (buf, "Home=", 5) == 0) {
                       	    strcpy (homedir, buf+5);
                       	    if (homedir[strlen (homedir)-1] != '/')
                       	        strcat (homedir, "/");
                	}
		    }
            	    fclose (tmp);

            } else {
            	fprintf(stderr, "Cannot open %s\n", fn);
            }
    	}
	if ( (p = getenv("LOGNAME")) != NULL ) {
        	struct passwd *pwd;
#ifdef __EMX__
	        if ( (pwd = _getpw(0, p)) != NULL ) {
#else
	        if ( (pwd = getpwnam(p, confdir)) != NULL ) {
#endif
			strcpy(userid, pwd->pw_name);
			strcpy(homedir, pwd->pw_dir);
        	}
    	}

    	/* news base directory */
    	if ((p = getenv("UUPCNEWS")) != NULL) {
    		SPOOLDIR = p;
/*             	if (SPOOLDIR [strlen(SPOOLDIR)-1 ] != '\\')
       			strcat(SPOOLDIR, "\\");*/
       	}

	sprintf(newsrc, "%s/.newsrc", homedir);
	sprintf(newnewsrc, "%s/.newnewsrc", homedir);
	sprintf(delgroups, "%s/.delgroups", homedir);
	sprintf(indexdir, "%s/.tindx", homedir);
	sprintf(active_file, "%s/active", LIBDIR);
	sprintf(signature, "%s/%s", homedir, sig);
	sprintf(sig, "%s/.Sig", homedir);

	if (my_stuff.replyto[0] == '\0')
		sprintf (my_stuff.replyto, "%s@%s", my_stuff.user,
						my_stuff.my_domain);
	if (mailer[0] == '\0')
		strcpy (mailer, MAILER);
	if (editor[0] == '\0')
		strcpy (editor, DEF_EDITOR);

	strcpy(my_from, my_stuff.replyto);
	strcat(my_from, "    (");
	strcat(my_from, my_stuff.my_name);
	strcat(my_from, ")");

#else
	myentry = getpwuid(getuid());
	strcpy(userid, myentry->pw_name);
	strcpy(homedir, myentry->pw_dir);

	sprintf(signature, "%s/.signature", homedir);
	sprintf(sig, "%s/.Sig", homedir);
	sprintf(newsrc, "%s/.newsrc", homedir);
	sprintf(newnewsrc, "%s/.newnewsrc", homedir);
	sprintf(delgroups, "%s/.delgroups", homedir);
	sprintf(indexdir, "%s/.tindx", homedir);
	strcpy(libdir, LIBDIR);

	sprintf(active_file, "%s/active", libdir);
	if (stat(active_file, &sb) >= 0)
		goto got_active;

/*
 *  I hate forgetting to define LIBDIR correctly.  Guess a
 *  couple of likely places if it's not where LIBDIR says it is.
 */

	strcpy(libdir, "/usr/lib/news");
	sprintf(active_file, "%s/active", libdir);
	if (stat(active_file, &sb) >= 0)
		goto got_active;

	strcpy(libdir, "/usr/local/lib/news");
	sprintf(active_file, "%s/active", libdir);
	if (stat(active_file, &sb) >= 0)
		goto got_active;

	strcpy(libdir, "/usr/public/lib/news");
	sprintf(active_file, "%s/active", libdir);
	if (stat(active_file, &sb) >= 0)
		goto got_active;

	strcpy(libdir, "/u/lib/news");
	sprintf(active_file, "%s/active", libdir);
	if (stat(active_file, &sb) >= 0)
		goto got_active;

/*
 *  Oh well.  Revert to what LIBDIR says it is to produce a
 *  useful error message when read_active() fails later.
 */

	sprintf(libdir, "%s", LIBDIR);
	sprintf(active_file, "%s/active", libdir);

got_active:

	*my_from = '\0';
	p = getenv("FROM");
	if (p != NULL)
		strcpy(my_from, p);

	*my_org = '\0';
	p = getenv("ORGANIZATION");
	if (p != NULL)
		strcpy(my_org, p);
	else {
		sprintf(nam, "%s/organization", libdir);
		fp = fopen(nam, "r");

		if (fp != NULL) {
			if (fgets(my_org, LEN, fp) != NULL) {
				for (p = my_org; *p && *p != '\n'; p++) ;
				*p = '\0';
			}
			fclose(fp);
		}
	}
#endif
}


char *
my_malloc(size)
unsigned size;
{
	char *p;

	p = malloc(size);
	if (p == NULL) {
		fprintf(stderr, "tass: out of memory\n");
		exit(1);
	}
	return p;
}


char *
my_realloc(p, size)
char *p;
unsigned size;
{
	if (p == NULL)
		p = malloc(size);
	else
		p = realloc(p, size);

	if (p == NULL) {
		fprintf(stderr, "tass: out of memory\n");
		exit(1);
	}
	return p;
}


char *
str_save(s)
char *s;
{
char *p;

	assert(s != NULL);
	
	p = my_malloc(strlen(s) + 1);
	strcpy(p, s);

	return(p);
}


copy_fp(a, b, prefix)
FILE *a;
FILE *b;
char *prefix;
{
	char buf[8192];

	while (fgets(buf, 8192, a) != NULL)
		fprintf(b, "%s%s", prefix, buf);
}


char *
get_val(env, def)
char *env;		/* Environment variable we're looking for	*/
char *def;		/* Default value if no environ value found	*/
{
	char *ptr;

	if ((ptr = getenv(env)) != NULL)
		return(ptr);
	else
		return(def);
}


invoke_editor(nam)
char *nam;
{
	char buf[LEN];
	static int first = TRUE;
#ifndef OS2
	static char editor[LEN];
#endif
	int ret;

	if (first) {
#ifndef OS2
		strcpy(editor, get_val("EDITOR", DEF_EDITOR));
#endif
		first = FALSE;
	}

#ifdef OS2
	sprintf(buf, editor, nam);/* editor definition includes %s */
#else
	sprintf(buf, "%s %s", editor, nam);
#endif
	printf("\r%s\n", buf);
	ret = invoke_cmd(buf);
	setuid(real_uid);
	setgid(real_gid);

	return ret;
}


invoke_cmd(nam)
char *nam;
{
	int ret;
#ifdef SIGTSTP
	void (*susp)();
#endif

	Raw(FALSE);
	setuid(real_uid);
	setgid(real_gid);

#ifdef SIGTSTP
	susp = signal(SIGTSTP, SIG_DFL);
#endif

	ret = system(nam);

#ifdef SIGTSTP
	signal(SIGTSTP, susp);
#endif

	setuid(tass_uid);
	setgid(tass_gid);
	Raw(TRUE);

	return ret == 0;
}


shell_escape() {
	char shell[LEN];
	char *p;
#ifdef SIGTSTP
	void (*susp)();
#endif

	if (!parse_string("!", shell))
#ifdef OS2
		strcpy(shell, get_val("SHELL", "cmd.exe"));
#else
		strcpy(shell, get_val("SHELL", "/bin/sh"));
#endif

	for (p = shell; *p && (*p == ' ' || *p == '\t'); p++) ;

	if (!*p)
#ifdef OS2
		strcpy(shell, get_val("SHELL", "cmd.exe"));
#else
		strcpy(shell, get_val("SHELL", "/bin/sh"));
#endif
	
	Raw(FALSE);

	setuid(real_uid);
	setgid(real_gid);

	fputs("\r\n", stdout);

#ifdef SIGTSTP
	susp = signal(SIGTSTP, SIG_DFL);
#endif

	system(p);

#ifdef SIGTSTP
	signal(SIGTSTP, susp);
#endif

	setuid(tass_uid);
	setgid(tass_gid);

	Raw(TRUE);

	continue_prompt();
	mail_setup();
}


add_signature(fp, flag)
FILE *fp;
int flag;
{
	FILE *sigf;

	sigf = fopen(signature, "r");
	if (sigf != NULL) {
#ifdef	OS2
		if (1) {
#else
		if (flag) {
#endif
			fprintf(fp, "\n--\n");
			copy_fp(sigf, fp, "");
		}
		fclose(sigf);
		return;
	}

	sigf = fopen(sig, "r");
	if (sigf != NULL) {
		fprintf(fp, "\n--\n");
		copy_fp(sigf, fp, "");
		fclose(sigf);
	}
}


make_lower(s, t)
char *s;
char *t;
{

	while (*s) {
		if (isupper(*s))
			*t = tolower(*s);
		else
			*t = *s;
		s++;
		t++;
	}
	*t = 0;
}


lcase(s)
char *s;
{

	while (*s)
	{
		if (isupper(*s))
			*s = tolower(*s);
		s++;
	}
}


match(s, t, n)
char *s;
char *t;
int n;
{

	while (*t) {
		if (*s == *t && strncmp(s, t, n) == 0)
			return TRUE;
		t++;
	}

	return FALSE;
}

