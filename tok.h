#ifndef VIMOL_TOK_H
#define VIMOL_TOK_H

/* string token */
typedef const char *tok_t;

/* token list */
struct tokq;

int tok_int(tok_t);
double tok_double(tok_t);
int tok_bool(tok_t);
const char *tok_string(tok_t);
color_t tok_color(tok_t);
vec_t tok_vec(tok_t);

struct tokq *tokq_create(const char *);
struct tokq *tokq_copy(struct tokq *, int, int);
void tokq_free(struct tokq *);
char *tokq_strcat(struct tokq *, int, int);
int tokq_count(struct tokq *);
tok_t tokq_tok(struct tokq *, int);

#endif /* VIMOL_TOK_H */
