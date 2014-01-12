/* raw.c */

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <exec/types.h>
#include <exec/ports.h>
#include <exec/memory.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>

/*
 *  This is a routine for setting stdin to raw or cooked mode on the
 *  Amiga . 
 *
 *  Written : 18-Jun-87 By Chuck McManis.
 *  Modified: 20-Apr-91 By Michael Taylor for stdin only
 */

/*
 * Function raw() - Convert stdin to 'raw' mode. This
 * essentially keeps DOS from translating keys for
 * you, also (BIG WIN) it means getch() will return immediately rather than
 * wait for a return. You lose editing features though.
 */

#define ACTION_SCREEN_MODE 994L
extern long SendPacket(	struct MsgPort *pid, long action, long args[],
			long nargs);
 
long
raw()
{
    struct MsgPort *mp; 	/* The File Handle message port */
    struct FileHandle *afh;
    long	    Arg[1], res;

    /* Step one, get the file handle */
    afh = (struct FileHandle *) (_IoStaticFD[fileno(stdin)].fd_Fh); 

    /* Step two, get it's message port. */
    mp = ((struct FileHandle *) (BADDR(afh)))->fh_Type;
    Arg[0] = -1L;
    res = SendPacket(mp, ACTION_SCREEN_MODE, Arg, 1);   /* Put it in RAW: mode */
    if (res == 0) {
	errno = ENXIO;
	return (-1);
    }
    return (0);
}

/*
 * Function - cooked() this function returns stdin to
 * it's normal, wait for a <CR> mode. This is exactly like raw() except that
 * it sends a 0 to the console to make it back into a CON: from a RAW:
 */

long
cooked()
{
    struct MsgPort *mp; 	/* The File Handle message port */
    struct FileHandle *afh;
    long	    Arg[1], res;

    /* Step one, get the file handle */
    afh = (struct FileHandle *) (_IoStaticFD[fileno(stdin)].fd_Fh); 

    /* Step two, get it's message port. */
    mp = ((struct FileHandle *) (BADDR(afh)))->fh_Type;
    Arg[0] = 0;
    res = SendPacket(mp, ACTION_SCREEN_MODE, Arg, 1);
    if (res == 0) {
	errno = ENXIO;
	return (-1);
    }
    return (0);
}

