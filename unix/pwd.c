/* pwd.c */

#include <stdio.h>
#include <string.h>
#include <pwd.h>

extern char * getenv (char *s);

int
getuid ()
{
	return (0);
}

int
getgid () {
	return (1);
}

int
getegid () {
	return (2);
}

int
geteuid () {
	return (3);
}

void setgid (int i) {
}

void setuid (int i) {
}

struct passwd *getpwuid(int uid)
{
  char *logname = getenv("LOGNAME");
  if (logname)
	return getpwnam(logname, "d:/uupc");
  else
  	return NULL;
}

struct passwd *getpwnam(char *name, char *confdir)
{
  static struct passwd pw;
  static char buffer[256];
  char *ptr, *logname, *fullname, *homedir;
  FILE *passwd;
  int i, found = 0;

  strcpy(buffer, confdir);
  strcat(buffer, "/passwd");

  if ( (passwd = fopen(buffer, "r")) == NULL )
    return NULL;

  while ( fgets(buffer, sizeof(buffer), passwd) != NULL )
  {
    buffer[strlen(buffer) - 1] = 0;

    if ( buffer[0] == '#' )
      continue;

    if ( (ptr = strchr(buffer, ':')) != NULL )
      *ptr++ = 0;
    else
      continue;

    if ( strcmp(name, buffer) == 0 )
    {
      logname = buffer;

      for ( i = 0; i < 3; i++ )
        if ( (ptr = strchr(ptr, ':')) != NULL )
          *ptr++ = 0;
        else
          continue;

      fullname = ptr;

      if ( (ptr = strchr(ptr, ':')) != NULL )
        *ptr++ = 0;
      else
        continue;

      homedir = ptr;

      if ( ptr[0] && ptr[1] && (ptr = strchr(ptr + 2, ':')) != NULL )
        *ptr++ = 0;   /* skip drive: */

      pw.pw_name  = logname;
      pw.pw_gecos = fullname;
      pw.pw_dir   = homedir;
      found = 1;

      break;
    }
  }

  fclose(passwd);

  return found ? &pw : NULL;
}

