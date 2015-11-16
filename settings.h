#ifndef VIMOL_SETTINGS_H
#define VIMOL_SETTINGS_H

void settings_init(void);
void settings_free(void);
int settings_printf(char *, size_t, const char *);
int settings_has_int(const char *);
int settings_has_double(const char *);
int settings_has_bool(const char *);
int settings_has_string(const char *);
int settings_has_color(const char *);
int settings_has_node(const char *);
int settings_get_int(const char *);
double settings_get_double(const char *);
int settings_get_bool(const char *);
const char *settings_get_string(const char *);
color_t settings_get_color(const char *);
int settings_set(const char *, const char *);
int settings_set_int(const char *, int);
int settings_set_double(const char *, double);
int settings_set_bool(const char *, int);
int settings_set_string(const char *, const char *);
int settings_set_color(const char *, color_t);

#endif /* VIMOL_SETTINGS_H */
