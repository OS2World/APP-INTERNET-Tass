/* link.c */

#include <stdio.h>

long
link (from, to)
char	*from, *to;
{
	return rename (from, to);
}
