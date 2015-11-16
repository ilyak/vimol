#ifndef VIMOL_PAIR_H
#define VIMOL_PAIR_H

struct pair {
	int i, j;
};

struct pairs;

struct pairs *pairs_create(void);
void pairs_free(struct pairs *);
void pairs_clear(struct pairs *);
void pairs_add(struct pairs *, int, int);
int pairs_get_count(struct pairs *);
struct pair pairs_get(struct pairs *, int);

#endif /* VIMOL_PAIR_H */
