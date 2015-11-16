#ifndef VIMOL_XSTR_H
#define VIMOL_XSTR_H

char *xstrcpy(char *, const char *);
char *xstrncpy(char *, const char *, size_t);
char *xstrcat(char *, const char *);
char *xstrncat(char *, const char *, size_t);
char *xstrdup(const char *);
char *xstrndup(const char *, size_t);

#endif /* VIMOL_XSTR_H */
