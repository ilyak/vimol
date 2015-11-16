#ifndef VIMOL_COLOR_H
#define VIMOL_COLOR_H

typedef struct color color_t;

struct color {
	double r, g, b;
};

color_t color_rgb(int, int, int);
int color_to_string(char *, size_t size, color_t);

#endif /* VIMOL_COLOR_H */
