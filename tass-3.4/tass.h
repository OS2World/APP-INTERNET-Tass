
#ifdef	OS2
extern char *	LIBDIR;
extern char *	SPOOLDIR;
#define         MAILER		"rmail"
#define         DEF_EDITOR	"emacs %s"
#else
#define         LIBDIR		"/usr/lib/news"
#define         SPOOLDIR	"/usr/spool/news"
#define         MAILER		"/bin/rmail"
#define         DEF_EDITOR	"/usr/bin/vi"
#endif

#define         TRUE		1
#define         FALSE		0

#define         LEN		512

#define         INDEX_TOP		4
#define         NOTESLINES		(LINES - INDEX_TOP - 2)
#define         RIGHT_POS		(COLS - 18)
#define         MORE_POS		(COLS - 20)

#define         MAX_SUBJ		(3 * (COLS-16) / 5)
#define         MAX_FROM		(COLS-17-MAX_SUBJ)

#define         TABLE_SIZE	1409		/* should be prime */


struct header {
	long artnum;
	char *subject;
	char *from;
	int thread;
	int inthread;
	int unread;		/* has this article been read? */
				/* 0 = read, 1 = unread, 2 = will return */
};

/*
 *  header.artnum:
 *	article number in spool directory for group
 *
 *  header.thread:
 *	initially -1
 *	points to another arts[] (struct header): zero and up
 *	-2 means article has expired (wasn't found in file search
 *	of spool directory for the group)
 *
 *  header.inthread:
 *	FALSE for the first article in a thread, TRUE for all
 *	following articles in thread
 *
 *  header.read:
 *	boolean, has this article been read or not
 */

struct group_ent {
	char *name;
	long max;
	long min;
	int next;		/* next active entry in hash chain */
	int flag;
};

/* this is the data we get from the UUPC .rc files */
typedef struct {
    char temp_name[256];	   /* unbatch temp file             */
    char news_dir[256];            /* news base directory	    */
    char incoming_dir[256];	   /* incoming news spool directory */
    char spool_dir[256];	   /* spool directory, log files    */
    char user[256];		   /* current user id		    */
    char my_name[256];		   /* my full name		    */
    char my_domain[256];	   /* our domain		    */
    char my_site[256];		   /* site name                     */
    char my_organisation[256];	   /* organisation		    */
    char mail_server[256];	   /* where posts are routed to     */
    char editor[256];		   /* system editor		    */
    char home[256];		   /* home mail directory	    */
    char signature[256];	   /* signature file		    */
    char replyto[256];		   /* reply to			    */
} INFO;

#define         NOTGOT		0x01	/* haven't put in my_group yet */
#define         SUBS		0x02	/* subscribed to */


extern int top;
extern struct header *arts;
extern long *base;
extern int max_art;

extern char sig[LEN];
extern char signature[LEN];
extern char userid[LEN];
extern char homedir[LEN];
extern char indexdir[LEN];
extern char my_org[LEN];
extern char my_from[LEN];
extern char active_file[LEN];
extern char newsrc[LEN];
extern char libdir[LEN];
extern char newnewsrc[LEN];
extern char delgroups[LEN];
extern int top_base;
extern int LINES, COLS;
extern char *str_save();
extern char *my_malloc();
extern char *my_realloc();
extern int group_hash[TABLE_SIZE];

extern int num_active;
extern struct group_ent *active;
extern int *my_group;
extern int *unread;
extern int max_active;

extern int local_top;
extern char *eat_re();
extern char *nice_time();
extern int update;
extern int inverse_okay;
extern char *h;
extern char cstate[];

extern int tass_uid;
extern int tass_gid;
extern int real_uid;
extern int real_gid;
extern int local_index;
extern int tass_backwards;

extern char *strcpy();
extern char *strncat();
extern char *strncpy();
extern long atol();


#define         ctrl(c)                 ((c) & 0x1F)

/*
 *  Assertion verifier
 */

#ifndef OS2
#ifdef __STDC__
#define assert(p)	if(! (p)) asfail(__FILE__, __LINE__, #p); else
#else
#define assert(p)	if(! (p)) asfail(__FILE__, __LINE__, "p"); else
#endif
#endif

#define         TASS_HEADER	"Tass 3.4x-4 (OS/2 version by Mike Taylor)"

#define XTRA		/* new features as of 3.4 */

#ifdef	OS2
/* archive.c */
FILE *open_archive_fp(char *group_path);
int archive_article(char *group_path);
int archive_thread(int respnum, char *group_path);

/* art.c */
void art_susp(int i);
int my_atol(char *s, int n);
int find_base(void);
int num_arts(void);
int valid_artnum(long art);
int purge_needed(void);
int index_group(char *group, char *group_path);
int read_group(char *group, char *group_path);
int make_threads(void);
char *eat_re(char *s);
int parse_headers(int fd, struct header *h);
int dump_index(char *group);
int my_strncpy(char *p, char *q, int n);
int load_index(void);
int find_local_index(char *group);
int do_update(void);

/* curses.c */
int InitScreen(void);
int ExitScreen(void);
int ScreenSize(int *lines, int *columns);
int ClearScreen(void);
int MoveCursor(int row, int col);
int CleartoEOLN(void);
int CleartoEOS(void);
int StartInverse(void);
int EndInverse(void);
int RawState(void);
int Raw(int state);
int ReadCh(void);
int outchar(int c);
int draw(char *p, char *q, int n);

/* group.c */
void group_susp(int i);
int group_page(char *group);
int fix_new_highest(int groupnum);
int show_group_page(char *group);
int draw_subject_arrow(void);
int erase_subject_arrow(void);
int prompt_subject_num(int ch, char *group);
int search_body(int current_art, char *group, char *group_path);
int search_art_body(char *ss, int len, long art, char *group_path);
int search_author(int current_art, int forward, char *group);
int search_subject(int forward, char *group);
int new_responses(int thread);

/* hashstr.c */
char *hash_str(char *s);
struct hashnode *add_string(char *s);
int hash_init(void);
int hash_reclaim(void);

/* help.c */
int tass_select_help(void);
int tass_group_help(void);
int tass_page_help(void);

/* mail.c */
int mail_setup(void);
int mail_check(void);

/* main.c */
void main_susp(int i);
int main(int argc, char **argv);
int tass_done(int ret);
int init_alloc(void);
int expand_art(void);
int expand_active(void);
int read_active(void);
int read_newsrc(int sub_only);
int write_newsrc(void);
int read_newsrc_line(char *group);
int update_newsrc(char *group, int groupnum);
int subscribe(char *group, int ch, int num, int out_seq);
int print_seq(FILE *fp, int groupnum);
int parse_seq(char *s);
int parse_unread(char *s, int groupnum);
int get_line_unread(char *group, int groupnum);
int reset_newsrc(void);
int delete_group(char *group);
int undel_group(void);
int mark_group_read(char *group, int groupnum);
long hash_groupname(char *buf);
int mkdir_p(char *path);
int switch_chars(int ch);

/* misc.c */
int which_base(int n);
int which_resp(int n);
int nresp(int n);
int prev_response(int n);
int next_response(int n);
int next_basenote(int n);
int next_unread(int n);
int prev_unread(int n);
int asfail(char *file, int line, char *cond);
int shell_escape();
int init_selfinfo();

/* nntp_open.c */
char *is_remote(void);
int nntp_startup(void);
int nntp_finish(void);
int get_respcode(void);
int stuff_nntp(char *fnam);
FILE *nntp_to_fp(void);
int nntp_to_fd(void);
FILE *open_active_fp(void);
FILE *open_art_fp(char *group_path, long art);
int open_header_fd(char *group_path, long art);
int nntp_stat(long artnum);
int setup_base(char *group, char *group_path);

/* page.c */
void page_susp(int i);
int show_page(int respnum, char *group, char *group_path);
int note_cleanup(void);
int note_save(void);
int note_reopen(long art, char *group_path);
int redraw_page(int respnum, char *group);
int show_note_page(int respnum, char *group);
int unrot(char *buf, char *buf2);
int show_first_header(int respnum, char *group);
int show_cont_header(int respnum);
int open_note(long art, char *group_path);
int prompt_response(int ch, int respnum);
int choose_resp(int i, int n);
int parse_from(char *str, char *addr, char *name);
int yank_to_addr(char *orig, char *addr);
int find_new_to(char *nam, char *mail_to);
int save_art_to_file(void);
int save_thread_to_file(int respnum, char *group_path);
int pipe_article(void);

/* post.c */
void log_post(char *nam);
int post_base(char *group);
int post_response(char *group, int copy_text);
int post_cc_response(char *group, int copy_text);
int mail_to_someone(int bounce);
int mail_to_author(int copy_text);
int post_cancel(char *group);

/* prompt.c */
int parse_num(int ch, char *prompt);
int parse_string(char *prompt, char *buf);
int prompt_yn(char *prompt);
int page_cont(void);
int continue_prompt(void);

/* screen.c */
int info_message(char *msg);
int clear_message(void);
int center_line(int line, char *str);
int draw_arrow(int line);
int erase_arrow(int line);
int ff1(int n, int ch);
int ff(int n);

/* select.c */
void select_susp(int i);
int selection_index(void);
int group_selection_page(void);
int prompt_group_num(int ch);
int erase_group_arrow(void);
int draw_group_arrow(void);
int search_group(int forward);
int choose_new_group(void);
int add_group(char *s, int get_unread);

/* spool_open.c */
char *is_remote(void);
int nntp_startup(void);
int nntp_finish(void);
FILE *open_active_fp(void);
FILE *open_art_fp(char *group_path, long art);
int open_header_fd(char *group_path, long art);
int base_comp(long *a, long *b);
int setup_base(char *group, char *group_path);

/* time.c */
int nicedate(char *timestr, char *newstr);
int nicetime(char *timestr, char *newstr);
char *nice_time(void);

/* ..\unix\CURSES.C */
int InitScreen(void);
int ScreenSize(int *lines, int *columns);
int ClearScreen(void);
int MoveCursor(int row, int col);
int Clr2EOLN(void);
int Clr2EOS(void);
int StartInverse(void);
int EndInverse(void);
int StartUnderline(void);
int EndUnderline(void);
int RawState(void);
int Raw(int state);
int ReadCh(void);
int outchar(int c);

/* ..\unix\link.c */
long link(char *from, char *to);

/* ..\unix\pwd.c */
int getuid(void);
int getgid(void);
int getegid(void);
int geteuid(void);
void setgid(int i);
void setuid(int i);
struct passwd *getpwuid(int uid);
#ifndef __EMX__
struct passwd *getpwnam(char *name, char *confdir);
#endif

/* ..\unix\reg.c */
int regfree(char *s);
char *regcmp(char *str);
char *regex(char *reg, char *str);

/* ..\unix\regcompat.c */
char *re_comp(char *str);
int re_exec(char *str);
int regerror(char *str);

/* ..\unix\regexp.c */
struct regexp *regcomp(char *exp);
int regexec(struct regexp *prog, char *string, int bolflag);
void regdump(struct regexp *r);

#ifndef __EMX__
/* ..\unix\termcap.c */
int tgetent(char *bp, char *name);
int tgetflag(char *id);
int tgetnum(char *id);
char *tgetstr(char *id, char **area);
char *do_esc(char *out, char *in);
char *tgoto(char *cm, int destcol, int destline);
void tputs(char *cp, int affcnt, int (*outc )(int ));
#endif

#endif

