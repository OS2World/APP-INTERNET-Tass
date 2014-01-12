18th May 1996
=============

Change release number to 3.4x-4.

Changed the subject arrow to be in inverse video.

10th January 1996
=================

Change release number to 3.4x-3.

Bug fix to reading of uupc variables. If a '=' was in a variable then it
was lost! e.g. Editor=vi "wq=8" %s

2nd September 1995 / 19 October 1995
====================================

Small changes to tass - change release number to 3.4x-1 (and then to 3.4x-2).

Changed the default behaviour of the tab key when at the group page. Now it
goes from unread thread to unread thread. The default was to go to next 
unread article. The old behaviour can be toggled by ctrl-T.

Changed linewrap to be on by default. this can be toggled by ctrl-W.

Fixed bug with post followup which also mailed to poster (only if a post 
followup and mail was done earlier!).

Fixed bug where Subject was lost on Group page.

Added local.rc so that tass can be used even with uupc installed. Modify the
local,rc and login.cmd files to reflect your requirements.

27th August 1995
================

Release of Tass for OS/2 version 3.4x. 

This is a port of tass 3.4 to OS/2. It is for use with UUPC (1.12k or higher).

I have set it up to use the UUPC configuration files. I have not changed
most of the unix style names so an HPFS formatted disk is required for
user home directories.

I ported the 3.4 sources from 1993 as I have been unable to find anything
newer.

The sources can be compiled with Borland C++ or with EMX (GCC) (The nntp 
sources can only be compiled with EMX).

I have tested it with uupc 1.12k

The supplied login.cmd file sets environment variables that tass/elm require.
It is used as follows
c:\> login userid


Michael Taylor
miket@pcug.org.au


My Notes
========

There are two executables
1. tass.exe - compiled with Borland C 2.0 for OS/2.
2. rtass.exe - compiled with emx/gcc. This is for nntp. You will require 
   the emx dlls to run it, get the file emxrt.zip from ftp-os2.nmsu.edu
   This has been tested but only a little as I use uupc. Also as tass builds
   index files for each group it can take a while to load a group page
   especially over a 

I have made a number of changes and enhancements:

1. It uses the UUPC configuration files for
   - user details
   - signature
   - editor name
   - some other things I have probably forgotten!

2. I added the '<' , '>' (and left and right arrow keys) to scroll a news
   article left and right. (I have made line wrap on by default so use Ctrl-W
   to toggle it off and then this will work)

3. I added the 'w/W' command to post a followup and mail for an article.

4. I changed it to use the rmail and inews command from uupc to post mail and 
   news. (this was simply a matter of removing the '/usr/' from the names in 
   the source code).
   
5. I added ctrl-W to line wrap an article (only at the article page display!)
   Unfortunately it appears some newer newsreaders have wordwrap but don't
   add cr/lfs to the posted article!
   
NOTES:

1. It must be used with a HPFS formatted drive for the user home directory 
   and the news directory (if using the uupc version) as it tries to use a 
   file called .newsrc! Also a few other illegal file names for FAT drives!
 
2. I have ported the nntp source by appropriating some nntp source from UQWK!
   I haven't got any idea what the posting programs would be so I haven't done
   the news/mail posting for nntp.
   

I have tested this myself for about ten months and it hasn't crashed for me for 
the last eight months at least.

All source code is included. I used the source code from ELM as a reference to 
get the curses.c working for OS/2 and the source code from UUQK to get the 
nntp stuff working (currently no support for posting news or sending mail with
nntp).

If anyone one knows Rich Skrenta please thank him for a nice newsreader (and a
easy program to port! - I have ported it to the Amiga, MSDOS and OS/2, although
I never solved all the 16bit-isms with MSDOS).

