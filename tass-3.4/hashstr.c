
#include	<stdio.h>


/*
 *  Maintain a table of all strings we have seen.
 *  If a new string comes in, add it to the table and return a pointer
 *  to it.  If we've seen it before, just return the pointer to it.
 *
 *  Usage:  hash_str("some string") returns char *
 *
 *  Spillovers are chained on the end
 */


/*
 *  Arbitrary table size, but make sure it's prime!
 */

/* #define		TABLE_SIZE	1409	*/

#define		TABLE_SIZE	2411



struct hashnode {
	char *s;			/* the string we're saving */
	struct hashnode *next;		/* chain for spillover */
};

struct hashnode *table[ TABLE_SIZE ];

extern char *my_malloc();
struct hashnode *add_string();


char *
hash_str(s)
char *s;
{
	struct hashnode *p;	/* used to descend the spillover structs */
	unsigned long h;	/* result of hash:  index into hash table */

	if (s == NULL)
		return(NULL);

	{
		char *t = s;

		h = *t++;
		while (*t)
			h = ((h << 1) ^ *t++) % TABLE_SIZE;
	/*		h = (h * 128 + *t++) % TABLE_SIZE;	*/
	}
	
	p = table[h];

	if (p == NULL) {
		table[h] = add_string(s);
		return table[h]->s;
	}

	while (1) {
		if (strcmp(s, p->s) == 0)
			return(p->s);

		if (p->next == NULL) {
			p->next = add_string(s);
			return p->next->s;
		} else
			p = p->next;
	}
}


struct hashnode *
add_string(s)
char *s;
{
	struct hashnode *p;
	extern char *strcpy();
	int *iptr;

	p = (struct hashnode *) my_malloc(sizeof(*p));

	p->next = NULL;
	iptr = (int *) my_malloc(strlen(s) + sizeof(int) + 1);
	*iptr++ = -1;
	p->s = (char *) iptr;
	strcpy(p->s, s);
	return(p);
}


hash_init() {
	int i;

	for (i = 0; i < TABLE_SIZE; i++)
		table[i] = NULL;
}


hash_reclaim() {
	int i;
	struct hashnode *p, *next;
	int *iptr;

	for (i = 0; i < TABLE_SIZE; i++)
		if (table[i] != NULL) {
			p = table[i];
			while (p != NULL) {
				next = p->next;
				iptr = (int *) p->s;
				free(--iptr);
				free(p);
				p = next;
			}
			table[i] = NULL;
		}
}

