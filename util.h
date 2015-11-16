#ifndef VIMOL_UTIL_H
#define VIMOL_UTIL_H

int util_is_empty(const char *);
int util_is_comment(const char *);
int util_file_exists(const char *);
int util_has_suffix(const char *, const char *);
char *util_next_line(char *, FILE *);

#endif /* VIMOL_UTIL_H */
