#ifndef VIMOL_ALIAS_H
#define VIMOL_ALIAS_H

/* alias management */
struct alias;

struct alias *alias_create(void);
void alias_free(struct alias *);
int alias_set(struct alias *, const char *, const char *);
const char *alias_get(struct alias *, const char *);

#endif /* VIMOL_ALIAS_H */
