#ifndef VIMOL_EXEC_H
#define VIMOL_EXEC_H

void exec_init(void);
void exec_free(void);
int exec_valid(const char *);
int exec_run(const char *, struct tokq *, struct state *);

#endif /* VIMOL_EXEC_H */
