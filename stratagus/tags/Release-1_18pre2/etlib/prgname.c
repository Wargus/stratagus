
const char *prgname = 0;

extern char *strrchr(const char *, int);

void
setprgname(char *str)
{
    char *x = strrchr(str, '/');

    prgname = x ? x+1 : str;
}
