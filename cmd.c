/*-
 * Copyright (c) 2013-2014 Ilya Kaliman
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
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
				return (FALSE);
			if (!parse_tokq(cmdq, atokq, alias, FALSE)) {
				tokq_free(atokq);
				return (FALSE);
			}
			tokq_free(atokq);
			continue;
		}

		if (!exec_valid(tok)) {
			error_set("invalid command \"%s\"", tok);
			return (FALSE);
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

	return (TRUE);
}

struct cmdq *
cmdq_from_string(const char *str, struct alias *alias)
{
	struct cmdq *cmdq;
	struct tokq *tokq;

	if ((tokq = tokq_create(str)) == NULL)
		return (NULL);

	cmdq = calloc(1, sizeof(*cmdq));
	cmdq->nalloc = 8;
	cmdq->data = calloc(cmdq->nalloc, sizeof(struct cmd));

	if (!parse_tokq(cmdq, tokq, alias, TRUE)) {
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
			return (FALSE);

	return (TRUE);
}
