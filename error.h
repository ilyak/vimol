#ifndef VIMOL_ERROR_H
#define VIMOL_ERROR_H

void error_set(const char *, ...);
const char *error_get(void);
void error_clear(void);

#endif /* VIMOL_ERROR_H */
