/*
  File: parse.c
  (Linux Access Control List Management)

  Copyright (C) 1999, 2000
  Andreas Gruenbacher, <a.gruenbacher@computer.org>
 	
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include "sys/acl.h"

#include "sequence.h"
#include "parse.h"
#include "misc.h"

#define SKIP_WS(x) ({ \
	while (*(x)==' ' || *(x)=='\t' || *(x)=='\n' || *(x)=='\r') \
		(x)++; \
	})


static int
skip_tag_name(
	const char **text_p,
	const char *token)
{
	size_t len = strlen(token);
	const char *text = *text_p;

	SKIP_WS(text);
	if (strncmp(text, token, len) == 0) {
		text += len;
		goto delimiter;
	}
	if (*text == *token) {
		text++;
		goto delimiter;
	}
	return 0;

delimiter:
	SKIP_WS(text);
	if (*text == ':') {
		*text_p = text+1;
		return 1;
	}
	if (*text == ',' || *text == '\0') {
		*text_p = text;
		return 1;
	}
	return 0;
}


static char *
get_token(
	const char **text_p)
{
	char *token = NULL, *t;
	const char *bp, *ep;

	bp = *text_p;
	SKIP_WS(bp);
	ep = bp;

	while (*ep!='\0' && *ep!='\r' && *ep!='\n' && *ep!=':' && *ep!=',')
		ep++;
	if (ep == bp)
		goto after_token;
	token = (char*)malloc(ep - bp + 1);
	if (token == NULL)
		goto after_token;
	memcpy(token, bp, ep - bp);

	/* Trim trailing whitespace */
	t = token + (ep - bp - 1);
	while (t >= token &&
	       (*t==' ' || *t=='\t' || *t=='\n' || *t=='\r'))
		t--;
	*(t+1) = '\0';

after_token:
	if (*ep == ':')
		ep++;
	*text_p = ep;
	return token;
}


static int
get_id(
	const char *token,
	id_t *id_p)
{
	char *ep;
	long l;
	l = strtol(token, &ep, 0);
	if (*ep != '\0')
		return -1;
	if (l < 0) {
		/*
		  Negative values are interpreted as 16-bit numbers,
		  so that id -2 maps to 65534 (nobody/nogroup), etc.
		*/
		l &= 0xFFFF;
	}
	*id_p = l;
	return 0;
}


static int
get_uid(
	const char *token,
	uid_t *uid_p)
{
	struct passwd *passwd;

	if (get_id(token, (id_t *)uid_p) == 0)
		goto accept;
	passwd = getpwnam(token);
	if (passwd) {
		*uid_p = passwd->pw_uid;
		goto accept;
	}
	return -1;

accept:
	return 0;
}


static int
get_gid(
	const char *token,
	gid_t *gid_p)
{
	struct group *group;

	if (get_id(token, (id_t *)gid_p) == 0)
		goto accept;
	group = getgrnam(token);
	if (group) {
		*gid_p = group->gr_gid;
		goto accept;
	}
	return -1;

accept:
	return 0;
}


/*
	Parses the next acl entry in text_p.

	Returns:
		-1 on error, 0 on success.
*/

cmd_t
parse_acl_cmd(
	const char **text_p,
	int seq_cmd,
	int parse_mode)
{
	cmd_t cmd = cmd_init();
	char *str;
	const char *backup;
	int error, perm_chars;
	if (!cmd)
		return NULL;

	cmd->c_cmd = seq_cmd;
	if (parse_mode & SEQ_PROMOTE_ACL)
		cmd->c_type = ACL_TYPE_DEFAULT;
	else
		cmd->c_type = ACL_TYPE_ACCESS;
	cmd->c_id   = ACL_UNDEFINED_ID;
	cmd->c_perm = 0;

	if (parse_mode & SEQ_PARSE_DEFAULT) {
		/* check for default acl entry */
		backup = *text_p;
		if (skip_tag_name(text_p, "default")) {
			if (parse_mode & SEQ_PROMOTE_ACL) {
				/* if promoting from acl to default acl and
				   a default acl entry is found, fail. */
				*text_p = backup;
				goto fail;
			}
			cmd->c_type = ACL_TYPE_DEFAULT;
		}
	}

	/* parse acl entry type */
	switch (**text_p) {
		case 'u':  /* user */
			skip_tag_name(text_p, "user");

user_entry:
			backup = *text_p;
			str = get_token(text_p);
			if (str) {
				cmd->c_tag = ACL_USER;
				error = get_uid(unquote(str), &cmd->c_id);
				free(str);
				if (error) {
					*text_p = backup;
					goto fail;
				}
			} else {
				cmd->c_tag = ACL_USER_OBJ;
			}
			break;

		case 'g':  /* group */
			if (!skip_tag_name(text_p, "group"))
				goto user_entry;

			backup = *text_p;
			str = get_token(text_p);
			if (str) {
				cmd->c_tag = ACL_GROUP;
				error = get_gid(unquote(str), &cmd->c_id); 
				free(str);
				if (error) {
					*text_p = backup;
					goto fail;
				}
			} else {
				cmd->c_tag = ACL_GROUP_OBJ;
			}
			break;

		case 'o':  /* other */
			if (!skip_tag_name(text_p, "other"))
				goto user_entry;
			/* skip empty entry qualifier field (this field may
			   be missing for compatibility with Solaris.) */
			SKIP_WS(*text_p);
			if (**text_p == ':')
				(*text_p)++;
			cmd->c_tag = ACL_OTHER;
			break;

		case 'm':  /* mask */
			if (!skip_tag_name(text_p, "mask"))
				goto user_entry;
			/* skip empty entry qualifier field (this field may
			   be missing for compatibility with Solaris.) */
			SKIP_WS(*text_p);
			if (**text_p == ':')
				(*text_p)++;
			cmd->c_tag = ACL_MASK;
			break;

		default:  /* assume "user:" */
			goto user_entry;
	}

	SKIP_WS(*text_p);
	if (**text_p == ',' || **text_p == '\0') {
		if (parse_mode & SEQ_PARSE_NO_PERM)
			return cmd;
		else
			goto fail;
	}
	if (!(parse_mode & SEQ_PARSE_WITH_PERM))
		return cmd;

	if (parse_mode & SEQ_PARSE_WITH_RELATIVE) {
		if (**text_p == '+') {
			(*text_p)++;
			cmd->c_cmd = CMD_ENTRY_ADD;
		} else if (**text_p == '^') {
			(*text_p)++;
			cmd->c_cmd = CMD_ENTRY_SUBTRACT;
		} else {
			if (!(parse_mode & SEQ_PARSE_NO_RELATIVE))
				goto fail;
		}
	}

	/* parse permissions */
	SKIP_WS(*text_p);
	if (**text_p >= '0' && **text_p <= '7') {
		cmd->c_perm = 0;
		while (**text_p == '0')
			(*text_p)++;
		if (**text_p >= '1' && **text_p <= '7') {
			cmd->c_perm = (*(*text_p)++ - '0');
		}

		return cmd;
	}

	for (perm_chars=0; perm_chars<3; perm_chars++, (*text_p)++) {
		switch(**text_p) {
			case 'r': /* read */
				if (cmd->c_perm & CMD_PERM_READ)
					goto fail;
				cmd->c_perm |= CMD_PERM_READ;
				break;

			case 'w':  /* write */
				if (cmd->c_perm & CMD_PERM_WRITE)
					goto fail;
				cmd->c_perm |= CMD_PERM_WRITE;
				break;

			case 'x':  /* execute */
				if (cmd->c_perm & CMD_PERM_EXECUTE)
					goto fail;
				cmd->c_perm |= CMD_PERM_EXECUTE;
				break;

			case 'X':  /* execute only if directory or some
				      entries already have execute permissions
				      set */
				if (cmd->c_perm & CMD_PERM_COND_EXECUTE)
					goto fail;
				cmd->c_perm |= CMD_PERM_COND_EXECUTE;
				break;

			case '-':
				/* ignore */
				break;

			default:
				if (perm_chars == 0)
					goto fail;
				return cmd;
		}
	}
	if (perm_chars != 3)
		goto fail;
	return cmd;

fail:
	cmd_free(cmd);
	return NULL;
}


/*
	Parse a comma-separated list of acl entries.

	which is set to the index of the first character that was not parsed,
	or -1 in case of success.
*/
int
parse_acl_seq(
	seq_t seq,
	const char *text_p,
	int *which,
	int seq_cmd,
	int parse_mode)
{
	const char *initial_text_p = text_p;
	cmd_t cmd;

	if (which)
		*which = -1;

	while (*text_p != '\0') {
		cmd = parse_acl_cmd(&text_p, seq_cmd, parse_mode);
		if (cmd == NULL) {
			errno = EINVAL;
			goto fail;
		}
		if (seq_append(seq, cmd) != 0) {
			cmd_free(cmd);
			goto fail;
		}
		SKIP_WS(text_p);
		if (*text_p != ',')
			break;
		text_p++;
	}

	if (*text_p != '\0') {
		errno = EINVAL;
		goto fail;
	}

	return 0;

fail:
	if (which)
		*which = (text_p - initial_text_p);
	return -1;
}



int
read_acl_comments(
	FILE *file,
	int *line,
	char **path_p,
	uid_t *uid_p,
	gid_t *gid_p)
{
	int c;
	char linebuf[1024];
	char *cp;
	char *p;
	int comments_read = 0;
	
	if (path_p)
		*path_p = NULL;
	if (uid_p)
		*uid_p = ACL_UNDEFINED_ID;
	if (gid_p)
		*gid_p = ACL_UNDEFINED_ID;

	for(;;) {
		c = fgetc(file);
		if (c == EOF)
			break;
		if (c==' ' || c=='\t' || c=='\r' || c=='\n') {
			if (c=='\n')
				(*line)++;
			continue;
		}
		if (c != '#') {
			ungetc(c, file);
			break;
		}
		if (line)
			(*line)++;

		if (fgets(linebuf, sizeof(linebuf), file) == NULL)
			break;
		
		comments_read = 1;

		p = strrchr(linebuf, '\0');
		while (p > linebuf &&
		       (*(p-1)=='\r' || *(p-1)=='\n')) {
		       	p--;
			*p = '\0';
		}
		
		cp = linebuf;
		SKIP_WS(cp);
		if (strncmp(cp, "file:", 5) == 0) {
			cp += 5;
			SKIP_WS(cp);
			cp = unquote(cp);
			
			if (path_p) {
				if (*path_p)
					goto fail;
				*path_p = (char*)malloc(strlen(cp)+1);
				if (!*path_p)
					return -1;
				strcpy(*path_p, cp);
			}
		} else if (strncmp(cp, "owner:", 6) == 0) {
			cp += 6;
			SKIP_WS(cp);
				
			if (uid_p) {
				if (*uid_p != ACL_UNDEFINED_ID)
					goto fail;
				if (get_uid(unquote(cp), uid_p) != 0)
					continue;
			}
		} else if (strncmp(cp, "group:", 6) == 0) {
			cp += 6;
			SKIP_WS(cp);
				
			if (gid_p) {
				if (*gid_p != ACL_UNDEFINED_ID)
					goto fail;
				if (get_gid(unquote(cp), gid_p) != 0)
					continue;
			}
		}
	}
	if (ferror(file))
		return -1;
	return comments_read;
fail:
	if (path_p && *path_p)
		free(*path_p);
	return -1;
}


int
read_acl_seq(
	FILE *file,
	seq_t seq,
	int seq_cmd,
	int parse_mode,
	int *line,
	int *which)
{
	char linebuf[1024];
	const char *cp;
	cmd_t cmd;

	if (which)
		*which = -1;

	for(;;) {
		if (fgets(linebuf, sizeof(linebuf), file) == NULL)
			break;
		if (line)
			(*line)++;

		cp = linebuf;
		SKIP_WS(cp);
		if (*cp == '\0') {
			if (!(parse_mode & SEQ_PARSE_MULTI))
				continue;
			break;
		} else if (*cp == '#') {
			continue;
		}

		cmd = parse_acl_cmd(&cp, seq_cmd, parse_mode);
		if (cmd == NULL) {
			errno = EINVAL;
			goto fail;
		}
		if (seq_append(seq, cmd) != 0) {
			cmd_free(cmd);
			goto fail;
		}

		SKIP_WS(cp);
		if (*cp != '\0' && *cp != '#') {
			errno = EINVAL;
			goto fail;
		}
	}

	if (ferror(file))
		goto fail;
	return 0;

fail:
	if (which)
		*which = (cp - linebuf);
	return -1;
}

