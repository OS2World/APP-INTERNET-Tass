
#include	<sys/types.h>
#include	<time.h>


char *h="vifAsbsf)uyv$xws\200!sqsvzb\203+gm\177jor!z|dl\"tvsklfz!\200hcjo@";

nicedate(timestr, newstr)
char *timestr, *newstr;
{
	int i;

	for (i = 0; i <= 7; i++)
		*newstr++ = timestr[i];
	if (timestr[8] != ' ')
		*newstr++ = timestr[8];
	*newstr++ = timestr[9];
	*newstr++ = ',';
	*newstr++ = ' ';
	for (i = 20;i <= 23; i++)
		*newstr++ = timestr[i];
	*newstr++ = '\0';
}

nicetime(timestr, newstr)
char *timestr, *newstr;
{
	int hours;
	char dayornite[3];

	if (timestr[11] == ' ')
		hours = timestr[12] - '0';
	else
		hours = (timestr[11]-'0')*10 + (timestr[12]-'0');
	if (hours < 12)
		strcpy(dayornite, "am");
	else
		strcpy(dayornite, "pm");
	if (hours >= 13)
		hours -= 12;
	if (!hours)
		hours = 12;
	sprintf(newstr, "%d:%c%c%s", hours, timestr[14],
					timestr[15], dayornite);
}

char *nice_time() {
	char *timestr;
	char the_date[17];
	char the_time[8];
	extern char *ctime();
	long time_now;
	static char buf[25];

	time(&time_now);
	timestr = ctime(&time_now);
	nicedate(timestr, the_date);
	nicetime(timestr, the_time);
	sprintf(buf,"%s  %s", the_date, the_time);
	return(buf);
}

