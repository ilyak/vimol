#ifndef VIMOL_CMD_H
#define VIMOL_CMD_H

/* vimol command */
struct cmd;

/* command list */
struct cmdq;

const char *cmd_name(struct cmd *);
struct tokq *cmd_args(struct cmd *);
int cmd_exec(struct cmd *, struct state *);

struct cmdq *cmdq_from_string(const char *, struct alias *);
void cmdq_free(struct cmdq *);
int cmdq_count(struct cmdq *);
struct cmd *cmdq_cmd(struct cmdq *, int);
int cmdq_exec(struct cmdq *, struct state *);

#endif /* VIMOL_CMD_H */
