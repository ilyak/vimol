#ifndef VIMOL_LOG_H
#define VIMOL_LOG_H

void log_open(const char *);
void log_warn(const char *, ...);
void log_fatal(const char *, ...) __dead2;
void log_close(void);

#endif /* VIMOL_LOG_H */
