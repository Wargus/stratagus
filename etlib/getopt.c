/*
 * Standard implementation of getopt(3).
 *
 * One extension: If the first character of the optionsstring is a ':'
 * the error return for 'argument required' is a ':' not a '?'.
 * This makes it easier to differentiate between an 'illegal option' and
 * an 'argument required' error.
 */

#define NULL	0
#define EOF	(-1)

extern int write();
extern int strlen();
extern int strcmp();
extern char *strchr();

int	opterr = 1;
int	optind = 1;
int	optopt;
char	*optarg;



static void
getopt_err(char *argv0, char *str, char opt)
{
    if (opterr)
    {
	char errbuf[2];
	char *x;

	errbuf[0] = opt;
	errbuf[1] = '\n';

	while ((x = strchr(argv0, '/')))
	    argv0 = x + 1;

	write(2, argv0, strlen(argv0));
	write(2, str, strlen(str));
	write(2, errbuf, 2);
    }
}



int
getopt(argc, argv, opts)
int	argc;
char	**argv, *opts;
{
    static int sp = 1;
    register int c;
    register char *cp;

    optarg = NULL;

    if (sp == 1)
    {
	if (optind >= argc || argv[optind][0] != '-' || argv[optind][1] == '\0')
	    return EOF;
	else if (strcmp(argv[optind], "--") == NULL)
	{
	    optind++;
	    return EOF;
	}
    }
    optopt = c = argv[optind][sp];
    if (c == ':' || (cp = strchr(opts, c)) == NULL)
    {
	getopt_err(argv[0], ": illegal option -", (char)c);
	cp = "xx";	/* make the next if false */
	c = '?';
    }
    if (*++cp == ':')
    {
	if (argv[optind][++sp] != '\0')
	    optarg = &argv[optind++][sp];
	else if (++optind < argc)
	    optarg = argv[optind++];
	else
	{
	    getopt_err(argv[0], ": option requires an argument -", (char)c);
	    c = (*opts == ':') ? ':' : '?';
	}
	sp = 1;
    }
    else if (argv[optind][++sp] == '\0')
    {
	optind++;
	sp = 1;
    }
    return c;
}
