#include <stdio.h>

/*
** printex - print fatal error message and exit.
*/

extern int errno;

/*
	error/abnormal condition cleanup and abort routine
	pass stack to printf
*/
printex (s,a,b,c,d,e,f)
char *s;
long a,b,c,d,e,f;
{
	static int topflag=0;
	if (topflag == 0)
	{
		++topflag;
		fflush (stdout);
		fprintf (stderr,s,a,b,c,d,e,f);
		fprintf (stderr," (error code %d)\n",errno);
		exit (1);
	}
	else
		fprintf (stderr,s,a,b,c,d,e,f);
}

extern char *malloc();

/*
        Storage allocaters.
*/

char *str_store (s)
char *s;
{
        static unsigned av_len = 0;     /* current storage available */
        static char *avail;
        int len;

        if (s == NULL)
                s = "";

        if ((len = strlen(s)+1) > av_len)
        {
                if (len > 1800)
                        av_len = len;
                else
                        av_len = 1800;
                if ((avail = malloc(av_len)) == NULL)
                        printex ("can't allocate memory for string storage");
        }
        strcpy (avail,s);
        s = avail;
        avail += len;
        av_len -= len;
        return (s);
}


