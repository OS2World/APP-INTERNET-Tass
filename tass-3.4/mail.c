
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

#define		TRUE		1
#define		FALSE		0


char *mailbox_name = NULL;
long mailbox_size;


/*
 *  Record size of mailbox so we can detect if new mail has arrived
 */

mail_setup() {
	struct stat buf;
	extern char *getenv();

	if (mailbox_name == NULL)
		mailbox_name = getenv("MAIL");

	if (mailbox_name == NULL)
		mailbox_size = 0;
	else {
		if (stat(mailbox_name, &buf) >= 0)
			mailbox_size = buf.st_size;
		else
			mailbox_size = 0;
	}
}


/*
 *  Return TRUE if new mail has arrived
 */

mail_check() {
	struct stat buf;

	if (mailbox_name != NULL
	&&  stat(mailbox_name, &buf) >= 0
	&&  mailbox_size < buf.st_size)
		return TRUE;

	return FALSE;
}

