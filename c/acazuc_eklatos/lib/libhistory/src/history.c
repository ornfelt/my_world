#include <readline/history.h>

#include <string.h>
#include <stdlib.h>

static HISTORY_STATE hist_state;
static HISTORY_STATE *g_state;

void using_history(void)
{
	g_state = &hist_state;
}

HISTORY_STATE *history_get_history_state(void)
{
	return g_state;
}

void history_set_history_state(HISTORY_STATE *state)
{
	g_state = state;
}

static HIST_ENTRY *entry_alloc(const char *line, const char *timestamp,
                               histdata_t data)
{
	HIST_ENTRY *entry = malloc(sizeof(*entry));
	if (!entry)
		return NULL;
	if (line)
	{
		entry->line = strdup(line);
		if (!entry->line)
		{
			free(entry);
			return NULL;
		}
	}
	else
	{
		entry->line = NULL;
	}
	if (timestamp)
	{
		entry->timestamp = strdup(timestamp);
		if (!entry->timestamp)
		{
			free(entry->line);
			free(entry);
			return NULL;
		}
	}
	else
	{
		entry->timestamp = NULL;
	}
	entry->data = data;
	return entry;
}

static void entry_free(HIST_ENTRY *entry)
{
	free(entry->line);
	free(entry->timestamp);
	free(entry);
}

void add_history(const char *line)
{
	HIST_ENTRY *entry = entry_alloc(line, NULL, NULL);
	if (!entry)
		return;
	if (g_state->flags & HS_STIFLED)
	{
		if (g_state->length >= g_state->size)
			free_history_entry(0);
	}
	if (g_state->length >= g_state->size)
	{
		int new_size = g_state->size * 2;
		if (new_size < 64)
			new_size = 64;
		HIST_ENTRY **entries = malloc(sizeof(*entries) * (new_size + 1));
		if (!entries)
		{
			entry_free(entry);
			return;
		}
		g_state->entries = entries;
		g_state->size = new_size;
	}
	g_state->entries[g_state->length++] = entry;
	g_state->entries[g_state->length] = NULL;
}

void add_history_time(const char *string)
{
	HIST_ENTRY *entry = history_get(g_state->length - 1);
	if (!entry)
		return;
	free(entry->timestamp);
	entry->timestamp = strdup(string);
}

HIST_ENTRY *remove_history(int which)
{
	if (which < 0 || which >= g_state->length)
		return NULL;
	HIST_ENTRY *entry = g_state->entries[which];
	memmove(&g_state->entries[which], &g_state->entries[which + 1],
	        sizeof(HIST_ENTRY*) * g_state->length - which);
	g_state->length--;
	return entry;
}

histdata_t free_history_entry(int which)
{
	HIST_ENTRY *entry = remove_history(which);
	if (!entry)
		return NULL;
	histdata_t data = entry->data;
	entry_free(entry);
	return data;
}

HIST_ENTRY *replace_history_entry(int which, const char *line,
                                  histdata_t data)
{
	if (which < 0 || which >= g_state->length)
		return NULL;
	HIST_ENTRY *entry = g_state->entries[which];
	HIST_ENTRY *new_entry = entry_alloc(line, NULL, data);
	g_state->entries[which] = new_entry;
	return entry;
}

void clear_history(void)
{
	for (int i = 0; i < g_state->length; ++i)
		entry_free(g_state->entries[i]);
	if (g_state->size)
		g_state->entries[0] = NULL;
	g_state->length = 0;
}

void stifle_history(int max)
{
	if (max < 0)
		max = 0;
	if (g_state->length > max)
	{
		int diff = g_state->length - max;
		for (int i = 0; i < diff; ++i)
			entry_free(g_state->entries[i]);
		memmove(&g_state->entries[0], &g_state->entries[diff],
		        sizeof(HIST_ENTRY*) * max + 1);
		g_state->length = max;
	}
	if (g_state->size != max)
	{
		if (max)
		{
			HIST_ENTRY **entries = realloc(g_state->entries,
			                               sizeof(*entries) * (max + 1));
			if (!entries)
				return;
			entries[max] = NULL;
			g_state->entries = entries;
		}
		else
		{
			free(g_state->entries);
			g_state->entries = NULL;
		}
		g_state->size = max;
	}
	g_state->flags |= HS_STIFLED;
}

int unstifle_history(void)
{
	if (!(g_state->flags & HS_STIFLED))
		return -1;
	g_state->flags &= ~HS_STIFLED;
	return g_state->size;
}

int history_is_stifled(void)
{
	return g_state->flags & HS_STIFLED;
}

HIST_ENTRY **history_list(void)
{
	return g_state->entries;
}

int where_history(void)
{
	return g_state->offset;
}

HIST_ENTRY *current_history(void)
{
	return g_state->entries[g_state->offset];
}

HIST_ENTRY *history_get(int offset)
{
	if (offset < 0 || offset >= g_state->length)
		return NULL;
	return g_state->entries[offset];
}

time_t history_get_time(HIST_ENTRY *entry)
{
	(void)entry;
	/* XXX */
	return 0;
}

int history_total_bytes(void)
{
	int sum = 0;
	for (int i = 0; i < g_state->length; ++i)
	{
		HIST_ENTRY *entry = g_state->entries[i];
		if (entry->line)
			sum += strlen(entry->line);
	}
	return sum;
}

int history_set_pos(int pos)
{
	if (pos < 0 || pos >= g_state->length)
		return -1;
	g_state->offset = pos;
	return 0;
}

HIST_ENTRY *previous_history(void)
{
	if (g_state->offset <= 0)
		return NULL;
	g_state->offset--;
	return g_state->entries[g_state->offset];
}

HIST_ENTRY *next_history(void)
{
	if (g_state->offset >= g_state->length)
		return NULL;
	g_state->offset++;
	return g_state->entries[g_state->offset];
}

static int entry_match(HIST_ENTRY *entry, const char *string)
{
	if (!entry->line)
		return -1;
	char *it = strstr(entry->line, string);
	if (!it)
		return -1;
	return it - entry->line;
}

int history_search(const char *string, int direction)
{
	return history_search_pos(string, direction, g_state->offset);
}

int history_search_prefix(const char *string, int direction)
{
	if (direction >= 0)
	{
		for (int i = g_state->offset; i < g_state->length; ++i)
		{
			HIST_ENTRY *entry = g_state->entries[i];
			int ret = entry_match(entry, string);
			if (ret == 0)
			{
				g_state->offset = i;
				return ret;
			}
		}
	}
	else
	{
		for (int i = g_state->offset; i > 0; --i)
		{
			HIST_ENTRY *entry = g_state->entries[i];
			int ret = entry_match(entry, string);
			if (ret == 0)
			{
				g_state->offset = i;
				return ret;
			}
		}
	}
	return -1;
}

int history_search_pos(const char *string, int direction, int pos)
{
	if (direction >= 0)
	{
		for (int i = pos; i < g_state->length; ++i)
		{
			HIST_ENTRY *entry = g_state->entries[i];
			int ret = entry_match(entry, string);
			if (ret >= 0)
			{
				g_state->offset = i;
				return ret;
			}
		}
	}
	else
	{
		for (int i = pos; i > 0; --i)
		{
			HIST_ENTRY *entry = g_state->entries[i];
			int ret = entry_match(entry, string);
			if (ret >= 0)
			{
				g_state->offset = i;
				return ret;
			}
		}
	}
	return -1;
}
