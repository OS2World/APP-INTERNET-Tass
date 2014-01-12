struct passwd {
	char	*pw_dir;
	char	*pw_name;
	char	*pw_gecos;
};

struct passwd *getpwnam(char *, char *);

