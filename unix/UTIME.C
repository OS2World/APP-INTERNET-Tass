#include <stdio.h>
#include <time.h>
#include <errno.h>

int utime (file, times)
char *file;
time_t times[2];
{
	unsigned long dateStamp[3];
	int result;
	unsigned long time1;

	if (!times) {
		result = SetFileDate(file, DateStamp (dateStamp));
		if (result == NULL) {
			errno = ENOENT;
			return -1;
		}
		return 0;
	}
	/* amiga time: base 01/01/1978          */
	
	dateStamp[0] = 	times[1] / (60 * 60 * 24);	/* number of days  */
	dateStamp[1] = 	times[1] / 60 -
			dateStamp[0] * 24 * 60;		/* minutes elapsed */
	dateStamp[2] =  0;				/* ticks elapsed   */

	result = SetFileDate(file, dateStamp);
	if (result == NULL) {
		errno = ENOENT;
		return -1;
	}
	
	return 0;
}
