#include <readline/readline.h>

#include <sys/param.h>
#include <sys/stat.h>

#include <termios.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>

#define MAX_KEYS 128

struct Keymap_sequence
{
	char *keyseq;
	int type;
	void *data;
};

struct Keymap
{
	Function *keys[MAX_KEYS];
	struct Keymap_sequence **sequences;
	size_t sequences_count;
};

static struct Keymap default_keymap;

char *rl_line_buffer;
int rl_point;
int rl_end;
int rl_mark;
int rl_done;
int rl_pending_input;
char *rl_prompt;
char *rl_terminal_name;
char *rl_readline_name;
FILE *rl_instream;
FILE *rl_outstream;
Function *rl_startup_hook;
Keymap rl_keymap = &default_keymap;
char *rl_basic_word_break_characters = " \t\n\"\\'`@$><=;|&{(";
Function *rl_completion_entry_function;

static char seq_buf[64];
static size_t seq_buf_len;

static struct termios prev_termios;
static int echoing = 1;

/* XXX remove usage of this function */
static void
repeat_str(const char *s, ssize_t n)
{
	for (ssize_t i = 0; i < n; ++i)
		fputs(s, rl_outstream);
}

static int
action_home(void)
{
	if (echoing)
	{
		repeat_str("\033[D", rl_point);
		fflush(rl_outstream);
	}
	rl_point = 0;
	return 0;
}

static int
action_delete(void)
{
	if (rl_point == rl_end)
		return 0;
	rl_delete_text(rl_point, rl_point + 1);
	return 0;
}

static int
action_end(void)
{
	if (echoing)
	{
		repeat_str("\033[C", rl_end - rl_point);
		fflush(rl_outstream);
	}
	rl_point = rl_end;
	return 0;
}

static int
action_backspace(void)
{
	if (rl_point <= 0)
		return 0;
	rl_delete_text(rl_point - 1, rl_point);
	return 0;
}

static int
action_forward(void)
{
	if (rl_point >= rl_end)
		return 0;
	rl_point++;
	if (echoing)
	{
		fputs("\033[C", rl_outstream);
		fflush(rl_outstream);
	}
	return 0;
}

static int
action_word_forward(void)
{
	size_t n = 0;

	while (rl_point < rl_end && strchr(rl_basic_word_break_characters,
	                                   rl_line_buffer[rl_point]))
	{
		rl_point++;
		n++;
	}
	while (rl_point < rl_end && !strchr(rl_basic_word_break_characters,
	                                    rl_line_buffer[rl_point]))
	{
		rl_point++;
		n++;
	}
	if (echoing)
	{
		repeat_str("\033[C", n);
		fflush(rl_outstream);
	}
	return 0;
}

static int
action_backward(void)
{
	if (rl_point <= 0)
		return 0;
	rl_point--;
	if (echoing)
	{
		fputs("\033[D", rl_outstream);
		fflush(rl_outstream);
	}
	return 0;
}

static int
action_word_backward(void)
{
	size_t n = 0;
	while (rl_point > 0 && strchr(rl_basic_word_break_characters,
	                              rl_line_buffer[rl_point - 1]))
	{
		rl_point--;
		n++;
	}
	while (rl_point > 0 && !strchr(rl_basic_word_break_characters,
	                               rl_line_buffer[rl_point - 1]))
	{
		rl_point--;
		n++;
	}
	if (echoing)
	{
		repeat_str("\033[D", n);
		fflush(rl_outstream);
	}
	return 0;
}

static int
action_kill(void)
{
	if (rl_point <= 0)
		return 0;
	rl_delete_text(0, rl_point);
	return 0;
}

static int
is_dir(DIR *dir, struct dirent *dirent)
{
	switch (dirent->d_type)
	{
		case DT_DIR:
			return 1;
		case DT_LNK:
		case DT_UNKNOWN:
		{
			struct stat st;
			if (fstatat(dirfd(dir), dirent->d_name, &st, 0) == -1)
				return -1;
			return S_ISDIR(st.st_mode) != 0;
		}
		default:
			return 0;
	}
}

char *
rl_filename_completion_function(char *text, int state)
{
	static char *prefix;
	static size_t prefix_len;
	static DIR *dir;
	static size_t dir_len;
	static char dir_path[MAXPATHLEN];
	static char buf[MAXPATHLEN * 2];
	size_t text_len = strlen(text);
	if (!state)
	{
		char *slash = memrchr(text, '/', text_len);
		if (slash)
		{
			if (slash == text)
			{
				dir_path[0] = '/';
				dir_path[1] = '\0';
				dir_len = 1;
			}
			else
			{
				memcpy(dir_path, text, slash - text);
				dir_path[slash - text] = '\0';
				dir_len = slash - text + 1;
			}
		}
		else
		{
			strlcpy(dir_path, ".", sizeof(dir_path));
			dir_len = 0;
		}
		dir = opendir(dir_path);
		if (!dir)
			return NULL;
		prefix = text + dir_len;
		prefix_len = strlen(prefix);
	}
	struct dirent *dirent;
	while ((dirent = readdir(dir)))
	{
		size_t name_len = dirent->d_reclen
		                - offsetof(struct dirent, d_name);
		if (name_len < prefix_len)
			continue;
		if (dirent->d_name[0] == '.' && prefix[0] != '.')
			continue;
		if (memcmp(dirent->d_name, prefix, prefix_len))
			continue;
		switch (is_dir(dir, dirent))
		{
			case -1:
				closedir(dir);
				return NULL;
			case 0:
				break;
			case 1:
				if (dir_len == 1 && dir_path[0] == '/')
					snprintf(buf, sizeof(buf), "/%s/",
					         dirent->d_name);
				else if (dir_len)
					snprintf(buf, sizeof(buf), "%s/%s/", dir_path,
					         dirent->d_name);
				else
					snprintf(buf, sizeof(buf), "%s/",
					         dirent->d_name);
				return buf;
		}
		if (dir_len == 1 && dir_path[0] == '/')
			snprintf(buf, sizeof(buf), "/%s", dirent->d_name);
		else if (dir_len)
			snprintf(buf, sizeof(buf), "%s/%s", dir_path,
			         dirent->d_name);
		else
			snprintf(buf, sizeof(buf), "%s", dirent->d_name);
		return buf;
	}
	closedir(dir);
	dir = NULL;
	return NULL;
}

static int
match_cmp(const void *a, const void *b)
{
	return strcmp(*(const char**)a, *(const char**)b);
}

char **
rl_completion_matches(char *text, Function *entry_func)
{
	char **matches = calloc(sizeof(*matches) * 2, 1);
	if (!matches)
		return NULL;
	size_t matches_count = 1;
	typedef char *(compl_fn_t)(char *text, int state);
	compl_fn_t *compl_fn = (compl_fn_t*)(void*)entry_func;
	char *compl;
	while ((compl = compl_fn(text, matches_count - 1)))
	{
		char **tmp = realloc(matches, sizeof(*tmp) * (matches_count + 2));
		if (!tmp)
			goto err;
		matches = tmp;
		matches[matches_count] = strdup(compl);
		if (!matches[matches_count])
			goto err;
		if (matches[0])
		{
			size_t i = 0;
			while (matches[matches_count][i]
			 && matches[0][i]
			 && matches[matches_count][i] == matches[0][i])
				++i;
			matches[0][i] = '\0';
		}
		else
		{
			matches[0] = strdup(compl);
			if (!matches[0])
				goto err;
		}
		matches_count++;
		matches[matches_count] = NULL;
	}
	qsort(&matches[1], matches_count - 1, sizeof(*matches), match_cmp);
	return matches;

err:
	if (matches)
	{
		for (size_t i = 0; i < matches_count; ++i)
			free(matches[i]);
		free(matches);
	}
	return NULL;
}

int
rl_complete_internal(int what)
{
	(void)what; /* XXX */
	if (rl_point < 0)
		return 0;
	char prefix[MAXPATHLEN];
	size_t prefix_len;
	if (rl_point > 0)
	{
		char *end = &rl_line_buffer[rl_point];
		char *start = end;
		while (start > rl_line_buffer && !strchr(rl_basic_word_break_characters,
		                                         *(start - 1)))
			start--;
		prefix_len = end - start;
		if (prefix_len >= sizeof(prefix))
			return 0;
		memcpy(prefix, start, prefix_len);
		prefix[prefix_len] = '\0';
	}
	else
	{
		prefix_len = 0;
		prefix[0] = '\0';
	}
	Function *compl_fn = rl_completion_entry_function;
	if (!compl_fn)
		compl_fn = (void*)rl_filename_completion_function;
	char **matches = rl_completion_matches(prefix, compl_fn);
	if (!matches)
		return 0;
	if (matches[0])
	{
		rl_delete_text(rl_point - prefix_len, rl_point);
		rl_insert_text(matches[0]);
	}
	if (!matches[1] || !matches[2])
		goto end;
	if (!echoing)
		goto end;
	int prev_point = rl_point;
	action_end();
	fprintf(rl_outstream, "\n%s",
	        matches[1]);
	for (size_t i = 2; matches[i]; ++i)
		fprintf(rl_outstream, " %s", matches[i]);
	fprintf(rl_outstream, "\n");
	fputs(rl_prompt, rl_outstream);
	if (rl_line_buffer)
		fputs(rl_line_buffer, rl_outstream);
	repeat_str("\033[D", rl_end - prev_point);
	rl_point = prev_point;
	fflush(rl_outstream);
end:
	for (size_t i = 0; matches[i]; ++i)
		free(matches[i]);
	free(matches);
	return 0;
}

int
rl_complete(int ignore, int invoking_key)
{
	(void)ignore;
	(void)invoking_key;
	return rl_complete_internal('\t');
}

int
rl_possible_completions(int count, int invoking_key)
{
	(void)count;
	(void)invoking_key;
	return rl_complete_internal('?');
}

int
rl_insert_completions(int count, int invoking_key)
{
	(void)count;
	(void)invoking_key;
	return rl_complete_internal('*');
}

__attribute__((constructor))
static void
rl_init(void)
{
	rl_bind_key('\x01', action_home);
	rl_bind_key('\x05', action_end);
	rl_bind_key('\b', action_backspace);
	rl_bind_key('\n', rl_on_new_line);
	rl_bind_key('\t', rl_complete);
	rl_bind_key('\x7F', action_backspace);
	rl_bind_key('\x15', action_kill);
	for (char c = ' '; c <= '~'; ++c)
		rl_bind_key(c, (Function*)rl_insert);
	rl_generic_bind(ISFUNC, "\033[C", action_forward, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[1;5C", action_word_forward, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[D", action_backward, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[1;5D", action_word_backward, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[1F", action_end, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[F", action_end, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[1H", action_home, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[1~", action_home, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[3~", action_delete, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[4~", action_end, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[7~", action_home, &default_keymap);
	rl_generic_bind(ISFUNC, "\033[8~", action_end, &default_keymap);
}

void
rl_deprep_terminal(void)
{
	tcsetattr(0, TCSANOW, &prev_termios);
}

void
rl_prep_terminal(int meta_flag)
{
	struct termios attr;

	tcgetattr(0, &attr);
	prev_termios = attr;
	if (meta_flag)
		attr.c_iflag &= ~ISTRIP;
	else
		attr.c_iflag |= ISTRIP;
	attr.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(0, TCSANOW, &attr);
}

int
rl_tty_set_echoing(int value)
{
	int prev = echoing;
	echoing = value;
	return prev;
}

int
rl_insert_text(char *text)
{
	size_t text_len = strlen(text);
	char *newl = realloc(rl_line_buffer, rl_end + text_len + 1);
	if (!newl)
		return 0;
	memmove(&newl[rl_point + text_len], &newl[rl_point], rl_end - rl_point);
	memcpy(&newl[rl_point], text, text_len);
	newl[rl_end + text_len] = '\0';
	if (echoing)
	{
		fputs(&newl[rl_point], rl_outstream);
		repeat_str("\033[D", rl_end - rl_point);
		fflush(rl_outstream);
	}
	rl_line_buffer = newl;
	rl_end += text_len;
	rl_point += text_len;
	return 0;
}

int
rl_delete_text(int start, int end)
{
	if (start < 0 || start >= rl_end)
		return 0;
	if (end <= start)
		return 0;
	int prev_end = rl_end;
	if (end > rl_end)
		end = rl_end;
	int diff = end - start;
	memmove(&rl_line_buffer[start], &rl_line_buffer[end], rl_end - end);
	rl_end -= diff;
	rl_line_buffer[rl_end] = '\0';
	int point_diff;
	if (rl_point >= end)
		point_diff = diff;
	else if (rl_point >= start)
		point_diff = rl_point - start;
	else
		point_diff = 0;
	rl_point -= point_diff;
	if (echoing)
	{
		repeat_str("\033[D", point_diff);
		fputs(&rl_line_buffer[start], rl_outstream);
		repeat_str(" ", prev_end - rl_end);
		repeat_str("\033[D", prev_end - rl_end);
		repeat_str("\033[D", rl_end - rl_point);
		fflush(rl_outstream);
	}
	return 0;
}

char *
rl_copy_text(int start, int end)
{
	if (start < 0)
		return NULL;
	if (end > rl_end)
		return NULL;
	if (start >= end)
		return NULL;
	char *str = malloc(end - start + 1);
	if (!str)
		return NULL;
	memcpy(str, &rl_line_buffer[start], end - start);
	str[end - start] = '\0';
	return str;
}

static int
process_char(char c)
{
	if (c == '\033')
	{
		seq_buf[0] = '\033';
		seq_buf_len = 1;
		return 0;
	}
	if (seq_buf_len == 1)
	{
		if (c == '[')
		{
			if (seq_buf_len != 1)
			{
				/* XXX */
				seq_buf_len = 0;
				return 0;
			}
			seq_buf[seq_buf_len++] = '[';
			return 0;
		}
		/* XXX commit prev? */
		seq_buf_len = 0;
		return 0;
	}
	if (seq_buf_len)
	{
		if (c == '\033')
		{
			/* XXX commit prev ? */
			seq_buf_len = 1;
			return 0;
		}
		if (seq_buf_len >= sizeof(seq_buf) - 2)
		{
			/* XXX ?? */
			seq_buf_len = 0;
			return 0;
		}
		seq_buf[seq_buf_len++] = c;
		if (isdigit(c) || c == ';')
			return 0;
		seq_buf[seq_buf_len++] = '\0';
		if (rl_keymap)
		{
			for (size_t i = 0; i < rl_keymap->sequences_count; ++i)
			{
				struct Keymap_sequence *sequence = rl_keymap->sequences[i];
				if (strcmp(sequence->keyseq, seq_buf))
					continue;
				switch (sequence->type)
				{
					case ISFUNC:
						((Function*)sequence->data)();
						break;
					default:
						/* XXX */
				}
				break;
			}
		}
		seq_buf_len = 0;
		return 0;
	}
	if (c > 0)
	{
		if (rl_keymap && rl_keymap->keys[(uint8_t)c])
		{
			rl_keymap->keys[(uint8_t)c](c);
			return 0;
		}
	}
	return 0;
}

char *
readline(const char *prompt)
{
	if (!rl_instream)
		rl_instream = stdin;
	if (!rl_outstream)
		rl_outstream = stdout;
	rl_prompt = (char*)prompt;
	rl_prep_terminal(0);
	rl_line_buffer = NULL;
	rl_end = 0;
	rl_point = 0;
	rl_done = 0;
	if (rl_startup_hook)
		rl_startup_hook();
	fputs(prompt, rl_outstream);
	fflush(rl_outstream);
	while (!rl_done)
	{
		int c;
		if (rl_pending_input)
		{
			c = rl_pending_input;
			rl_pending_input = 0;
		}
		else
		{
			c = getc(rl_instream);
			if (c == EOF)
				break;
		}
		if (process_char(c))
		{
			free(rl_line_buffer);
			rl_line_buffer = NULL;
			goto end;
		}
	}
	if (!rl_line_buffer)
	{
		rl_line_buffer = strdup("");
		if (!rl_line_buffer)
			goto end;
	}
end:
	rl_deprep_terminal();
	return rl_line_buffer;
}

int
rl_insert(char c)
{
	char tmp[2];
	tmp[0] = c;
	tmp[1] = '\0';
	rl_insert_text(tmp);
	return 0;
}

int
rl_on_new_line(void)
{
	if (echoing)
	{
		fprintf(rl_outstream, "\n");
		fflush(rl_outstream);
	}
	rl_done = 1;
	return 0;
}

Keymap
rl_make_bare_keymap(void)
{
	Keymap map = malloc(sizeof(*map));
	if (!map)
		return NULL;
	memset(map, 0, sizeof(*map));
	return map;
}

Keymap
rl_copy_keymap(Keymap map)
{
	Keymap dup = malloc(sizeof(*map));
	if (!dup)
		return NULL;
	dup->sequences = malloc(sizeof(*dup->sequences) * map->sequences_count);
	if (!dup->sequences)
	{
		free(dup);
		return NULL;
	}
	for (size_t i = 0; i < map->sequences_count; ++i)
	{
		struct Keymap_sequence *sequence = map->sequences[i];
		struct Keymap_sequence *dup_seq = malloc(sizeof(*dup_seq));
		if (!dup_seq)
		{
			for (size_t j = 0; j < i; ++j)
			{
				free(dup->sequences[j]->keyseq);
				free(dup->sequences[j]);
			}
			free(dup->sequences);
			free(dup);
			return NULL;
		}
		dup_seq->type = sequence->type;
		dup_seq->data = sequence->data;
		dup_seq->keyseq = strdup(sequence->keyseq);
		if (!dup_seq->keyseq)
		{
			for (size_t j = 0; j < i; ++j)
			{
				free(dup->sequences[j]->keyseq);
				free(dup->sequences[j]);
			}
			free(dup_seq);
			free(dup->sequences);
			free(dup);
			return NULL;
		}
		dup->sequences[i] = dup_seq;
	}
	dup->sequences_count = map->sequences_count;
	memcpy(dup->keys, map->keys, sizeof(dup->keys));
	return dup;
}

Keymap
rl_make_keymap(void)
{
	Keymap map = rl_make_bare_keymap();
	if (!map)
		return NULL;
	rl_bind_key_in_map('\n', rl_on_new_line, map);
	for (char c = ' '; c < '~'; ++c)
		rl_bind_key_in_map(c, (Function*)rl_insert, map);
	return map;
}

void
rl_discard_keymap(Keymap keymap)
{
	if (!keymap)
		return;
	for (size_t i = 0; i < keymap->sequences_count; ++i)
	{
		free(keymap->sequences[i]->keyseq);
		free(keymap->sequences[i]);
	}
	free(keymap->sequences);
	free(keymap);
}

Keymap
rl_get_keymap(void)
{
	return rl_keymap;
}

void
rl_set_keymap(Keymap keymap)
{
	rl_keymap = keymap;
}

Keymap
rl_get_keymap_by_name(const char *name)
{
	(void)name;
	/* XXX */
	return NULL;
}

int
rl_bind_key(int key, Function *function)
{
	return rl_bind_key_in_map(key, function, rl_keymap);
}

int
rl_bind_key_in_map(int key, Function *function, Keymap map)
{
	if (key <= 0 || key >= MAX_KEYS)
		return 1;
	map->keys[key] = function;
	return 0;
}

int
rl_unbind_key(int key)
{
	return rl_unbind_key_in_map(key, rl_keymap);
}

int
rl_unbind_key_in_map(int key, Keymap map)
{
	return rl_bind_key_in_map(key, NULL, map);
}

int
rl_generic_bind(int type, const char *keyseq, void *data, Keymap map)
{
	if (type != ISFUNC)
		return 1; /* XXX */
	struct Keymap_sequence *sequence = malloc(sizeof(*sequence));
	if (!sequence)
		return 1;
	sequence->keyseq = strdup(keyseq);
	if (!sequence->keyseq)
	{
		free(sequence);
		return 1;
	}
	sequence->type = type;
	sequence->data = data;
	struct Keymap_sequence **sequences = realloc(map->sequences,
	                                             sizeof(*sequences) * (map->sequences_count + 1));
	if (!sequences)
	{
		free(sequence->keyseq);
		free(sequence);
		return 1;
	}
	sequences[map->sequences_count++] = sequence;
	map->sequences = sequences;
	return 0;
}
