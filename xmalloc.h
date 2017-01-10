#ifndef VIMOL_XMALLOC_H
#define VIMOL_XMALLOC_H

void *xcalloc(size_t, size_t);
char *xstrcpy(char *, const char *);
char *xstrcat(char *, const char *);
char *xstrdup(const char *);
char *xstrndup(const char *, size_t);

#endif /* VIMOL_XMALLOC_H */
