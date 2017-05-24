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

static void
cmdq_push_back(struct cmdq *cmdq, struct cmd cmd)
{
	if (cmdq->nelts == cmdq->nalloc) {
		cmdq->nalloc *= 2;
		cmdq->data = xrealloc(cmdq->data,
		    cmdq->nalloc * sizeof *cmdq->data);
	}
	cmdq->data[cmdq->nelts++] = cmd;
}

static int
parse_tokq(struct cmdq *cmdq, struct tokq *tokq)
{
	struct cmd cmd;
	int argc, i;
	const char *tok;

	for (i = 0; i < tokq_count(tokq); i++) {
		tok = tok_string(tokq_tok(tokq, i));

		if (strcmp(tok, ";") == 0)
			continue;

		if (!exec_is_valid(tok)) {
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

static void
cmdq_free(struct cmdq *cmdq)
{
	int i;

	if (cmdq) {
		for (i = 0; i < cmdq->nelts; i++) {
			free(cmdq->data[i].name);
			tokq_free(cmdq->data[i].args);
		}
		free(cmdq->data);
		free(cmdq);
	}
}

static struct cmdq *
cmdq_from_string(const char *str)
{
	struct cmdq *cmdq;
	struct tokq *tokq;

	if ((tokq = tokq_create(str)) == NULL)
		return (NULL);

	cmdq = xcalloc(1, sizeof *cmdq);
	cmdq->nalloc = 8;
	cmdq->data = xcalloc(cmdq->nalloc, sizeof *cmdq->data);

	if (!parse_tokq(cmdq, tokq)) {
		cmdq_free(cmdq);
		tokq_free(tokq);
		return (NULL);
	}

	tokq_free(tokq);
	return (cmdq);
}

static int
cmdq_exec(struct cmdq *cmdq, struct state *state)
{
	struct cmd *cmd;
	int i;

	for (i = 0; i < cmdq->nelts; i++) {
		cmd = &cmdq->data[i];
		if (!exec_run(cmd->name, cmd->args, state))
			return (0);
	}
	return (1);
}

int
cmd_is_valid(const char *str)
{
	struct cmdq *cmdq;

	if ((cmdq = cmdq_from_string(str)) == NULL)
		return (0);
	cmdq_free(cmdq);
	return (1);
}

int
cmd_exec(const char *str, struct state *state)
{
	struct cmdq *cmdq;
	int rc;

	if ((cmdq = cmdq_from_string(str)) == NULL)
		return (0);
	rc = cmdq_exec(cmdq, state);
	cmdq_free(cmdq);
	return (rc);
}
