/* opendir.c
 * =========
 *
 * unix-like opendir and closedir commands
 *
 */

#include <stdio.h>
#include <stdlib.h>
 
#include <dirent.h>

static DIR dirent[MAXDIRS];
static int i = -1;
	
DIR * opendir ( dirname )
char * dirname;
{
	long lck;
	int j;

	if (i == -1) {
		for (j = 0; j < MAXDIRS; dirent[j++].lck = 0L);
		i = 0;
	} else {
		if (++i >= MAXDIRS) {
			for (j = 0; j < MAXDIRS; ++j)
				if (dirent[j].lck == 0)
					break;
			if (j < MAXDIRS)
				i = j;
		}
		if (i >= MAXDIRS)
			return (0L);
	}
	
	dirent[i].fp = (struct FileInfoBlock *)
       		malloc ((long) sizeof (struct FileInfoBlock));
	
	dirent[i].lck = Lock (dirname, ACCESS_READ);
	Examine (dirent[i].lck, dirent[i].fp);

	return (&dirent[i]);
}

struct dirent * readdir (d)
DIR * d;
{
	if (ExNext (d->lck, d->fp) == 0)
		return (NULL);
	d->e.d_name = d->fp->fib_FileName;
	d->e.d_reclen = strlen (d->fp->fib_FileName);

	return (&d->e);
}

void closedir (d)
DIR * d;
{
		UnLock (d->lck);
		free (d->fp);
		d->lck = 0L;
		d->fp = 0L;
}

