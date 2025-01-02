#ifndef READLINE_READLINE_H
#define READLINE_READLINE_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ISFUNC 1
#define ISMACR 2
#define ISKMAP 3

typedef int Function();
typedef void VFunction();
typedef char *CPFunction();
typedef char **CPPFunction();

typedef struct Keymap *Keymap;

extern char *rl_line_buffer;
extern int rl_point;
extern int rl_end;
extern int rl_mark;
extern int rl_done;
extern int rl_pending_input;
extern char *rl_prompt;
extern char *rl_terminal_name;
extern char *rl_readline_name;
extern FILE *rl_instream;
extern FILE *rl_outstream;
extern Function *rl_startup_hook;
extern Keymap rl_keymap;
extern char *rl_basic_word_break_characters;
extern Function *rl_completion_entry_function;

char *readline(const char *prompt);

int rl_insert_text(char *text);
int rl_delete_text(int start, int end);
char *rl_copy_text(int start, int end);

void rl_prep_terminal(int meta_flag);
void rl_deprep_terminal(void);
int rl_tty_set_echoing(int value);

int rl_insert(char c);
int rl_on_new_line(void);
char *rl_filename_completion_function(char *text, int state);
char **rl_completion_matches(char *text, Function *entry_func);
int rl_complete_internal(int what);
int rl_complete(int ignore, int invoking_key);
int rl_possible_completions(int count, int invoking_key);
int rl_insert_completions(int count, int invoking_key);

Keymap rl_make_bare_keymap(void);
Keymap rl_copy_keymap(Keymap map);
Keymap rl_make_keymap(void);
void rl_discard_keymap(Keymap keymap);
Keymap rl_get_keymap(void);
void rl_set_keymap(Keymap keymap);
Keymap rl_get_keymap_by_name(const char *name);

int rl_bind_key(int key, Function *function);
int rl_bind_key_in_map(int key, Function *function, Keymap map);
int rl_unbind_key(int key);
int rl_unbind_key_in_map(int key, Keymap map);
int rl_generic_bind(int type, const char *keyseq, void *data, Keymap map);

#ifdef __cplusplus
}
#endif

#endif
