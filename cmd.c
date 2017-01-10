/*
 * Copyright (c) 2013-2017 Ilya Kaliman
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "vimol.h"

struct cmd {
	char *name;
	struct tokq *args;
};

struct cmdq {
	int nelts, nalloc;
	struct cmd *data;
};

const char *
cmd_name(struct cmd *cmd)
{
	return (cmd->name);
}

struct tokq *
cmd_args(struct cmd *cmd)
{
	return (cmd->args);
}

int
cmd_exec(struct cmd *cmd, struct state *state)
{
	return (exec_run(cmd->name, cmd->args, state));
}

static void
cmdq_push_back(struct cmdq *cmdq, struct cmd cmd)
{
	if (cmdq->nelts == cmdq->nalloc) {
		cmdq->nalloc *= 2;
		cmdq->data = realloc(cmdq->data,
		    cmdq->nalloc * sizeof(struct cmd));
	}
	cmdq->data[cmdq->nelts++] = cmd;
}

static int
parse_tokq(struct cmdq *cmdq, struct tokq *tokq, struct alias *alias,
    int expand)
{
	struct cmd cmd;
	struct tokq *atokq;
	int argc, i;
	const char *tok, *astr;

	for (i = 0; i < tokq_count(tokq); i++) {
		tok = tok_string(tokq_tok(tokq, i));

		if (strcmp(tok, ";") == 0)
			continue;

		if (expand && (astr = alias_get(alias, tok))) {
			if ((atokq = tokq_create(astr)) == NULL)
				return (0);
			if (!parse_tokq(cmdq, atokq, alias, 0)) {
				tokq_free(atokq);
				return (0);
			}
			tokq_free(atokq);
			continue;
		}

		if (!exec_valid(tok)) {
			error_set("invalid command \"%s\"", tok);
			return (0);
		}

		cmd.name = xstrdup(tok);

		for (argc = 0; i + argc + 1 < tokq_count(tokq); argc++) {
			tok = tok_string(tokq_tok(tokq, i + argc + 1));

			if (strcmp(tok, ";") == 0)
				break;
		}

		cmd.args = tokq_copy(tokq, i + 1, argc);
		cmdq_push_back(cmdq, cmd);

		i = i + argc + 1;
	}

	return (1);
}

struct cmdq *
cmdq_from_string(const char *str, struct alias *alias)
{
	struct cmdq *cmdq;
	struct tokq *tokq;

	if ((tokq = tokq_create(str)) == NULL)
		return (NULL);

	cmdq = xcalloc(1, sizeof(*cmdq));
	cmdq->nalloc = 8;
	cmdq->data = xcalloc(cmdq->nalloc, sizeof(struct cmd));

	if (!parse_tokq(cmdq, tokq, alias, 1)) {
		cmdq_free(cmdq);
		tokq_free(tokq);
		return (NULL);
	}

	tokq_free(tokq);
	return (cmdq);
}

void
cmdq_free(struct cmdq *cmdq)
{
	int i;

	for (i = 0; i < cmdq_count(cmdq); i++) {
		free(cmdq->data[i].name);
		tokq_free(cmdq->data[i].args);
	}

	free(cmdq->data);
	free(cmdq);
}

int
cmdq_count(struct cmdq *cmdq)
{
	return (cmdq->nelts);
}

struct cmd *
cmdq_cmd(struct cmdq *cmdq, int idx)
{
	assert(idx >= 0 && idx < cmdq_count(cmdq));

	return (&cmdq->data[idx]);
}

int
cmdq_exec(struct cmdq *cmdq, struct state *state)
{
	int i;

	for (i = 0; i < cmdq_count(cmdq); i++)
		if (!cmd_exec(cmdq_cmd(cmdq, i), state))
			return (0);

	return (1);
}
