#ifndef READLINE_HISTORY_H
#define READLINE_HISTORY_H

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

#define HS_STIFLED (1 << 0)

typedef void *histdata_t;

typedef struct _hist_entry
{
	char *line;
	char *timestamp;
	histdata_t data;
} HIST_ENTRY;

typedef struct _hist_state
{
	HIST_ENTRY **entries;
	int offset;
	int length;
	int size;
	int flags;
} HISTORY_STATE;

extern int history_base;
extern int history_length;
extern int history_max_entries;

void using_history(void);
HISTORY_STATE *history_get_history_state(void);
void history_set_history_state(HISTORY_STATE *state);

void add_history(const char *line);
void add_history_time(const char *string);
HIST_ENTRY *remove_history(int which);
histdata_t free_history_entry(int which);
HIST_ENTRY *replace_history_entry(int which, const char *line, histdata_t data);
void clear_history(void);

void stifle_history(int max);
int unstifle_history(void);
int history_is_stifled(void);

HIST_ENTRY **history_list(void);
int where_history(void);
HIST_ENTRY *current_history(void);
time_t history_get_time(HIST_ENTRY *entry);
HIST_ENTRY *history_get(int offset);
int history_total_bytes(void);

int history_set_pos(int pos);
HIST_ENTRY *previous_history(void);
HIST_ENTRY *next_history(void);

int history_search(const char *string, int direction);
int history_search_prefix(const char *string, int direction);
int history_search_pos(const char *string, int direction, int pos);

#ifdef __cplusplus
}
#endif

#endif
