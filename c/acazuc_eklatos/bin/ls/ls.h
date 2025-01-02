#ifndef LS_H
#define LS_H

#include <sys/queue.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stddef.h>
#include <dirent.h>

struct dir
{
	const char *path;
	const char *name;
	DIR *dir;
	int links_len;
	int user_len;
	int group_len;
	int size_len;
	int date_len;
	int total_links;
	int ino_len;
	int blocks_len;
	TAILQ_HEAD(, file) files;
	TAILQ_ENTRY(dir) chain;
};

struct file
{
	char *name;
	char *lnk_name;
	char perms[12];
	char user[33];
	char group[33];
	char size[32];
	char date[16];
	char links[32];
	char ino[32];
	char blocks[32];
	mode_t mode;
	mode_t lnk_mode;
	time_t sort_date;
	off_t sort_size;
	TAILQ_ENTRY(file) chain;
};

struct source
{
	char *path;
	char *display_path;
	int sort_date;
	off_t sort_size;
	TAILQ_ENTRY(source) chain;
};

#define OPT_a (1 << 0)
#define OPT_A (1 << 1)
#define OPT_b (1 << 2)
#define OPT_c (1 << 3)
#define OPT_d (1 << 4)
#define OPT_F (1 << 5)
#define OPT_G (1 << 6)
#define OPT_h (1 << 7)
#define OPT_H (1 << 8)
#define OPT_i (1 << 9)
#define OPT_l (1 << 10)
#define OPT_L (1 << 11)
#define OPT_m (1 << 12)
#define OPT_n (1 << 13)
#define OPT_N (1 << 14)
#define OPT_o (1 << 15)
#define OPT_p (1 << 16)
#define OPT_q (1 << 17)
#define OPT_Q (1 << 18)
#define OPT_r (1 << 19)
#define OPT_R (1 << 20)
#define OPT_s (1 << 21)
#define OPT_S (1 << 22)
#define OPT_t (1 << 23)
#define OPT_u (1 << 24)
#define OPT_U (1 << 25)
#define OPT_1 (1 << 26)
#define OPT_x (1 << 27)

struct env
{
	const char *progname;
	int opt;
	int printed_file;
	TAILQ_HEAD(, source) sources;
};

void parse_sources(struct env *env, int ac, char **argv);
void print_dir(struct env *env, const char *path, int is_recur, const char *display_path);
void print_file(struct env *env, struct file *file, struct dir *dir, int last);
void dir_add_file(struct env *env, struct dir *dir, const char *name);
void dir_init(struct dir *dir, const char *path);
struct dir *load_dir(struct env *env, const char *path);
void free_file(struct file *file);
int load_file(struct env *env, struct file *file, const char *name, struct dir *dir);
time_t file_time(const struct env *env, const struct stat *st);
void print_sources(struct env *env, int recur);

#endif
