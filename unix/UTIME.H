#ifndef _UTIME_H
#define _UTIME_H

struct utimbuf {
	int a;
	int b;
}

int utime (char * pathname, struct utimbuf *times);

#endif
