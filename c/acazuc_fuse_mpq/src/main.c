#define FUSE_USE_VERSION 26

#include <wow/mpq.h>

#include <sys/queue.h>

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <fuse.h>

//#define DEBUG

struct mpq_node
{
	char *name;
	TAILQ_HEAD(, mpq_node) childs;
	TAILQ_ENTRY(mpq_node) chain;
	struct mpq_node *parent;
};

struct mpq_fs
{
	const char *mpq_fn;
	struct wow_mpq_compound *mpq;
	struct wow_mpq_archive *archive;
	struct mpq_node *root;
};

static void node_get_path(struct mpq_node *node, char *str, size_t len)
{
	if (!node->parent)
	{
		snprintf(str, len, "%s", node->name);
		return;
	}
	node_get_path(node->parent, str, len);
	size_t pos = strlen(str);
	if (str[0])
	{
		snprintf(str + pos, len - pos, "\\");
		pos++;
	}
	snprintf(str + pos, len - pos, "%s", node->name);
}

static struct mpq_node *get_node(struct mpq_fs *fs, const char *path)
{
	struct mpq_node *parent = fs->root;
	const char *prev = path;
	const char *pos;
	while ((pos = strchr(prev, '/')))
	{
		if (pos == prev)
			goto next_iter;
		char dir[512];
		snprintf(dir, sizeof(dir), "%.*s", (int)(pos - prev), prev);
		for (size_t i = 0; dir[i]; ++i)
			dir[i] = tolower(dir[i]);
		struct mpq_node *node;
		TAILQ_FOREACH(node, &parent->childs, chain)
		{
			if (!strcmp(node->name, dir))
			{
				parent = node;
				goto next_iter;
			}
		}
		return NULL;
next_iter:
		prev = pos + 1;
	}
	size_t rem = strlen(path) - (prev - path);
	if (!rem)
		return parent;
	char file[512];
	snprintf(file, sizeof(file), "%s", prev);
	size_t len = strlen(file);
	for (size_t i = 0; i < len; ++i)
		file[i] = tolower(file[i]);
	struct mpq_node *node;
	TAILQ_FOREACH(node, &parent->childs, chain)
	{
		if (!strcmp(node->name, file))
			return node;
	}
	return NULL;
}

static int mpq_getattr(const char *path, struct stat *st)
{
	struct fuse_context *ctx = fuse_get_context();
	struct mpq_fs *fs = ctx->private_data;
	struct mpq_node *node = get_node(fs, path);
#ifdef DEBUG
	printf("getattr %s %p\n", path, node);
#endif
	if (!node)
		return -ENOENT;
	memset(st, 0, sizeof(*st));
	if (TAILQ_EMPTY(&node->childs))
	{
		char path[512];
		node_get_path(node, path, sizeof(path));
		const struct wow_mpq_block *block = wow_mpq_get_block(fs->mpq, path);
		if (!block)
			return -ENOENT;
		st->st_mode = S_IFREG | 0444;
		st->st_size = block->file_size;
	}
	else
	{
		st->st_mode = S_IFDIR | 0555;
		st->st_size = 0;
	}
	st->st_nlink = 1;
	return 0;
}

static int mpq_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
                       off_t offset, struct fuse_file_info *fi)
{
	struct fuse_context *ctx = fuse_get_context();
	struct mpq_fs *fs = ctx->private_data;
	struct mpq_node *node = get_node(fs, path);
#ifdef DEBUG
	printf("readdir %s %p\n", path, node);
#endif
	if (!node)
		return -ENOENT;
	if (TAILQ_EMPTY(&node->childs))
		return -ENOENT;
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);
	struct mpq_node *child;
	TAILQ_FOREACH(child, &node->childs, chain)
		filler(buf, child->name, NULL, 0);
	return 0;
}

static int mpq_open(const char *path, struct fuse_file_info *info)
{
	struct fuse_context *ctx = fuse_get_context();
	struct mpq_fs *fs = ctx->private_data;
	struct mpq_node *node = get_node(fs, path);
#ifdef DEBUG
	printf("open %s %p\n", path, node);
#endif
	if (!node || !TAILQ_EMPTY(&node->childs))
		return -ENOENT;
	if ((info->flags & O_ACCMODE) != O_RDONLY)
		return -EACCES;
	return 0;
}

static int mpq_read(const char *path, char *buf, size_t size, off_t offset,
                    struct fuse_file_info *info)
{
	struct fuse_context *ctx = fuse_get_context();
	struct mpq_fs *fs = ctx->private_data;
	struct mpq_node *node = get_node(fs, path);
#ifdef DEBUG
	printf("read %s %p\n", path, node);
#endif
	/* XXX cache */
	char mpq_path[512];
	node_get_path(node, mpq_path, sizeof(mpq_path));
	struct wow_mpq_file *file = wow_mpq_get_file(fs->mpq, mpq_path);
	if (!file)
		return -ENOENT;
	if (wow_mpq_seek(file, offset, SEEK_SET) < 0)
	{
		wow_mpq_file_delete(file);
		return -ENOENT;
	}
	uint32_t rd = wow_mpq_read(file, buf, size);
	wow_mpq_file_delete(file);
	return rd;
}

static const struct fuse_operations fuse_op =
{
	.getattr = mpq_getattr,
	.readdir = mpq_readdir,
	.open = mpq_open,
	.read = mpq_read,
};

static struct mpq_node *mpq_node_new(const char *name, struct mpq_node *parent)
{
	struct mpq_node *node = malloc(sizeof(*node));
	if (!node)
		return NULL;
	TAILQ_INIT(&node->childs);
	node->name = strdup(name);
	node->parent = parent;
	if (!node->name)
	{
		free(node);
		return NULL;
	}
	return node;
}

static bool add_mpq_file(struct mpq_fs *fs, const char *path)
{
	struct mpq_node *parent = fs->root;
	struct mpq_node *new_node;
	const char *prev = path;
	const char *pos;
	while ((pos = strchr(prev, '\\')))
	{
		if (pos == prev)
		{
			pos++;
			continue;
		}
		char dir[512];
		snprintf(dir, sizeof(dir), "%.*s", (int)(pos - prev), prev);
		for (size_t i = 0; dir[i]; ++i)
			dir[i] = tolower(dir[i]);
		struct mpq_node *node;
		TAILQ_FOREACH(node, &parent->childs, chain)
		{
			if (!strcmp(node->name, dir))
			{
				parent = node;
				goto next_iter;
			}
		}
		new_node = mpq_node_new(dir, parent);
		if (!new_node)
		{
			fprintf(stderr, "failed to create node\n");
			return false;
		}
		TAILQ_INSERT_TAIL(&parent->childs, new_node, chain);
		parent = new_node;
next_iter:
		pos++;
		prev = pos;
	}
	size_t rem = strlen(path) - (prev - path);
	if (rem > 0)
	{
		char file[512];
		snprintf(file, sizeof(file), "%s", prev);
		size_t len = strlen(file);
		for (size_t i = 0; i < len; ++i)
			file[i] = tolower(file[i]);
		struct mpq_node *node;
		TAILQ_FOREACH(node, &parent->childs, chain)
		{
			if (!strcmp(node->name, file))
			{
				parent = node;
				return true;
			}
		}
		struct mpq_node *new_node = mpq_node_new(file, parent);
		if (!new_node)
		{
			fprintf(stderr, "failed to create node\n");
			return false;
		}
		TAILQ_INSERT_TAIL(&parent->childs, new_node, chain);
	}
	return true;
}

static bool load_files(struct mpq_fs *fs)
{
	/* XXX create popup to diplay loading files */
	fs->root = mpq_node_new("", NULL);
	struct wow_mpq_compound *compound = fs->mpq;
	struct wow_mpq_file *file = wow_mpq_get_file(fs->mpq, "(listfile)");
	if (!file)
	{
		fprintf(stderr, "no listfile\n");
		return false;
	}
	const char *prev = (const char*)file->data;
	const char *pos;
	while ((pos = (const char*)memchr(prev, '\n', file->size - (prev - (const char*)file->data))))
	{
		if (pos == prev)
		{
			pos++;
			continue;
		}
		char path[256];
		snprintf(path, sizeof(path), "%.*s", (int)(pos - prev), prev);
		size_t len = strlen(path);
		if (path[len - 1] == '\r')
			path[len - 1] = '\0';
		if (!add_mpq_file(fs, path))
			fprintf(stderr, "failed to add mpq file\n");
		pos++;
		prev = pos;
	}
	wow_mpq_file_delete(file);
	return true;
}

int main(int argc, char **argv)
{
	int ret;
	struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
	struct mpq_fs fs;
	const struct fuse_opt opts[] =
	{
		{"--mpq=%s", offsetof(struct mpq_fs, mpq_fn), 1},
		FUSE_OPT_END,
	};
	if (fuse_opt_parse(&args, &fs, opts, NULL) == -1)
		return EXIT_FAILURE;
	if (!fs.mpq_fn)
	{
		fprintf(stderr, "%s: no mpq given\n", argv[0]);
		return EXIT_FAILURE;
	}
	fs.mpq = wow_mpq_compound_new();
	if (!fs.mpq)
	{
		fprintf(stderr, "%s: mpq allocation failed\n", argv[0]);
		return EXIT_FAILURE;
	}
	fs.archive = wow_mpq_archive_new(fs.mpq_fn);
	if (!fs.archive)
	{
		fprintf(stderr, "%s: failed to open archive\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!wow_mpq_compound_add_archive(fs.mpq, fs.archive))
	{
		fprintf(stderr, "%s: failed to load archive\n", argv[0]);
		return EXIT_FAILURE;
	}
	if (!load_files(&fs))
		return EXIT_FAILURE;
	ret = fuse_main(args.argc, args.argv, &fuse_op, &fs);
	fuse_opt_free_args(&args);
	return ret;
}
