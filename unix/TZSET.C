long timezone = 28800;
int daylight = 1;
char *tzname[2] = {"PST", "PDT"};

void tzset () {
	timezone = 0;
	daylight = 0;
	memcpy (tzname[0], "EST", 3);
	memcpy (tzname[1], "EST", 3);
}
