/* regcompat.c */
/* file: regcompat.c
** author: Peter S. Housel 11/21/88
** Compatibility routines for regular expressions. more.c uses the
** re_comp() and re_exec() routines, while Minix only has regcomp() and
** regexec() (from Henry Spencer's freely redistributable regexp package).
** Note that the third argument to regexec() is a beginning-of-line flag
** and was probably added by Andrew Tannenbaum. It will probably be ignored
** if your copy of the regexp routines only expects two args.
**/

#include <regexp.h>
/* following just for debugging #define NULL	0 */

#include <stdio.h>

extern char *malloc();

static regexp *re_exp = NULL;	/* currently compiled regular expression */
static char *re_err = NULL;	/* current regexp error */

char *re_comp(str)
char *str;
{
 if(str == NULL)
    return NULL;

 if(re_exp != NULL)
    free(re_exp);

 if((re_exp = regcomp(str)) != NULL)
    return NULL;

 return re_err != NULL ? re_err : "string didn't compile";
}

int re_exec(str)
char *str;
{
 if(re_exp == NULL)
    return -1;
 return regexec(re_exp, str, 1);
}

regerror(str)
char *str;
{
 re_err = str;
}
