#include "../utils/shaders.h"

#include "display.h"

#include <wow/bls.h>

#include <inttypes.h>
#include <stdbool.h>

struct bls_display
{
	struct display display;
	struct wow_bls_file *file;
};

static void on_gtk_block_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data);

static void dtr(struct display *ptr)
{
	struct bls_display *display = (struct bls_display*)ptr;
	wow_bls_file_delete(display->file);
}

struct display *bls_display_new(const struct node *node, const char *path, struct wow_mpq_file *mpq_file)
{
	(void)node;
	(void)path;
	struct wow_bls_file *file = wow_bls_file_new(mpq_file);
	if (!file)
	{
		fprintf(stderr, "failed to parse bls file\n");
		return NULL;
	}
	struct bls_display *display = calloc(sizeof(*display), 1);
	if (!display)
	{
		fprintf(stderr, "bls display allocation failed\n");
		wow_bls_file_delete(file);
		return NULL;
	}
	display->display.name = strdup(path);
	display->display.dtr = dtr;
	display->file = file;
	/* shaders tree */
	GtkTreeStore *store = gtk_tree_store_new(2, G_TYPE_STRING, G_TYPE_INT);
	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	g_signal_connect(tree, "row-activated", G_CALLBACK(on_gtk_block_row_activated), display);
	gtk_tree_view_set_activate_on_single_click(GTK_TREE_VIEW(tree), true);
	GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes("blocks", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	gtk_widget_set_vexpand(tree, true);
	GtkTreeIter block_iter;
	GtkTreeIter shader_iter;
	for (size_t i = 0; i < file->blocks_nb; ++i)
	{
		const char *ps_block_names[12] =
		{
			"ps_1.1",
			"ps_1.2",
			"ps_1.3",
			"ps_1.4",
			"ps_2.0",
			"nv_register_shader",
			"nv_texture_shader1",
			"nv_texture_shader2",
			"nv_texture_shader3",
			"ATIfs",
			"ARBfp1.0",
			"unknown" /* not handled in code */
		};
		const char *vs_block_names[6] =
		{
			"vs_1.1",
			"vs_2.0",
			"ARBvp1.0",
			"unknown", /* unhandled but opengl */
			"unknown", /* unhandled but opengl */
			"unknown", /* unhandled in code */
		};
		const char *name;
		if (file->header.header.magic == (('S' << 0) | ('V' << 8) | ('X' << 16) | ('G' << 24)))
			name = vs_block_names[i];
		else
			name = ps_block_names[i];
		gtk_tree_store_append(store, &block_iter, NULL);
		gtk_tree_store_set(store, &block_iter, 0, name, 1, i, -1);
		for (size_t j = 0; j < file->blocks[i].shaders_nb; ++j)
		{
			char shader_name[128];
			snprintf(shader_name, sizeof(shader_name), "permutation %d", (int)j);
			gtk_tree_store_append(store, &shader_iter, &block_iter);
			gtk_tree_store_set(store, &shader_iter, 0, shader_name, 1, ((i + 1) << 16) | (j + 1), -1);
		}
	}
	gtk_widget_show(tree);
	/* Scrolled */
	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_vexpand(scrolled, true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), tree);
	gtk_widget_show(scrolled);
	/* Paned */
	GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_start_child(GTK_PANED(paned), scrolled);
	gtk_paned_set_position(GTK_PANED(paned), 200);
	gtk_widget_show(paned);
	display->display.root = paned;
	return &display->display;
}

static GtkWidget *build_gtk_code(struct bls_display *display, const struct wow_bls_shader *shader, bool pixel, uint32_t block)
{
	(void)display;
	GtkTextBuffer *buffer = gtk_text_buffer_new(NULL);
	if (pixel)
	{
		switch (block)
		{
			case WOW_BLS_PIXEL_PS_1_1:
			case WOW_BLS_PIXEL_PS_1_2:
			case WOW_BLS_PIXEL_PS_1_3:
			case WOW_BLS_PIXEL_PS_1_4:
			case WOW_BLS_PIXEL_PS_2_0:
			{
				char buf[1024 * 16] = "";
				decode_dx9_shader(buf, sizeof(buf), shader->code, shader->code_len);
				gtk_text_buffer_set_text(buffer, buf, strlen(buf));
				break;
			}
			case WOW_BLS_PIXEL_NV_REGISTER:
			{
				char buf[1024 * 16] = "";
				decode_nv_register_shader(buf, sizeof(buf), shader->code, shader->code_len);
				gtk_text_buffer_set_text(buffer, buf, strlen(buf));
				break;
			}
			case WOW_BLS_PIXEL_NV_TEXTURE1:
			case WOW_BLS_PIXEL_NV_TEXTURE2:
			case WOW_BLS_PIXEL_NV_TEXTURE3:
			{
				char buf[1024 * 16] = "";
				decode_nv_register_shader(buf, sizeof(buf), shader->code, shader->code_len);
				uint32_t rem = shader->code_len >= 0x614 ? shader->code_len - 0x614 : 0;
				size_t cur_len = strlen(buf);
				decode_nv_texture_shader(buf + cur_len, sizeof(buf) - cur_len, shader->code + 0x614, rem);
				gtk_text_buffer_set_text(buffer, buf, strlen(buf));
				break;
			}
			case WOW_BLS_PIXEL_ARBFP1_0:
			{
				gtk_text_buffer_set_text(buffer, (const char*)shader->code, shader->code_len);
				break;
			}
			default:
			{
				static const char *str = "unhandled block type";
				gtk_text_buffer_set_text(buffer, str, strlen(str));
				break;
			}
		}
	}
	else
	{
		switch (block)
		{
			case WOW_BLS_VERTEX_VS_1_1:
			case WOW_BLS_VERTEX_VS_2_0:
			{
				char buf[1024 * 16] = "";
				decode_dx9_shader(buf, sizeof(buf), shader->code, shader->code_len);
				gtk_text_buffer_set_text(buffer, buf, strlen(buf));
				break;
			}
			case WOW_BLS_VERTEX_ARBVP1_0:
			{
				gtk_text_buffer_set_text(buffer, (const char*)shader->code, shader->code_len);
				break;
			}
			default:
			{
				static const char *str = "unhandled block type";
				gtk_text_buffer_set_text(buffer, str, strlen(str));
				break;
			}
		}
	}
	GtkWidget *text = gtk_text_view_new_with_buffer(buffer);
	gtk_text_view_set_monospace(GTK_TEXT_VIEW(text), true);
	gtk_widget_show(text);
	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), text);
	gtk_widget_show(scrolled);
	return scrolled;
}

static GtkWidget *build_gtk_params(struct bls_display *display, const struct wow_bls_shader_param *params, uint32_t params_count)
{
	(void)display;
	GtkListStore *store = gtk_list_store_new(6, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
	gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(tree), true);
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkTreeViewColumn *column;
	column = gtk_tree_view_column_new_with_attributes("name", renderer, "text", 0, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("binding", renderer, "text", 1, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("mat4", renderer, "text", 2, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("type", renderer, "text", 3, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("unk1", renderer, "text", 4, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	column = gtk_tree_view_column_new_with_attributes("unk2", renderer, "text", 5, NULL);
	gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);
	GtkTreeIter iter;
	for (uint32_t i = 0; i < params_count; ++i)
	{
		static const char *types[] =
		{
			"vec4",
			"mat3x4",
			"mat4x4",
			"texture",
			"bump mat",
			"vec3",
			"vec2",
			"vec1",
			"mat3x3",
			"struct",
			"array",
		};
		const struct wow_bls_shader_param *param = &params[i];
		char binding[32];
		char mat4[256];
		char unk1[32];
		char unk2[32];
		snprintf(binding, sizeof(binding), "%" PRIu32, param->binding);
		snprintf(mat4, sizeof(mat4), "{%f, %f, %f, %f}\n{%f, %f, %f, %f}\n{%f, %f, %f, %f}\n{%f, %f, %f, %f}", param->f[0], param->f[1], param->f[2], param->f[3], param->f[4], param->f[5], param->f[6], param->f[7], param->f[8], param->f[9], param->f[10], param->f[11], param->f[12], param->f[13], param->f[14], param->f[15]);
		snprintf(unk1, sizeof(unk1), "%" PRIu32, param->unk1);
		snprintf(unk2, sizeof(unk2), "%" PRIu32, param->unk2);
		gtk_list_store_append(store, &iter);
		gtk_list_store_set(store, &iter, 0, param->name, 1, binding, 2, mat4, 3, types[param->type], 4, unk1, 5, unk2, -1);
	}
	gtk_widget_show(tree);
	GtkWidget *scrolled = gtk_scrolled_window_new();
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_widget_set_vexpand(scrolled, true);
	gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled), tree);
	gtk_widget_show(scrolled);
	return scrolled;
}

static GtkWidget *build_display(struct bls_display *display, const struct wow_bls_shader *shader, bool pixel, uint32_t block)
{
	GtkWidget *code = build_gtk_code(display, shader, pixel, block);
	GtkWidget *consts = build_gtk_params(display, shader->consts, shader->consts_nb);
	GtkWidget *params = build_gtk_params(display, shader->params, shader->params_nb);
	/* Args */
	GtkWidget *args = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
	gtk_paned_set_start_child(GTK_PANED(args), consts);
	gtk_paned_set_end_child(GTK_PANED(args), params);
	gtk_widget_show(args);
	/* Box */
	GtkWidget *paned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
	gtk_paned_set_start_child(GTK_PANED(paned), code);
	gtk_paned_set_end_child(GTK_PANED(paned), args);
	gtk_paned_set_position(GTK_PANED(paned), 500);
	gtk_widget_show(paned);
	return paned;
}

static void on_gtk_block_row_activated(GtkTreeView *tree, GtkTreePath *path, GtkTreeViewColumn *column, gpointer data)
{
	struct bls_display *display = data;
	GtkTreeIter iter;
	GtkTreeModel *model = gtk_tree_view_get_model(tree);
	gtk_tree_model_get_iter(GTK_TREE_MODEL(model), &iter, path);
	uint32_t val;
	gtk_tree_model_get(GTK_TREE_MODEL(model), &iter, 1, &val, -1);
	if (!(val & 0xFFFF0000))
		return;
	uint32_t block = (val >> 16) - 1;
	GtkWidget *child = build_display(display, &display->file->blocks[block].shaders[(val & 0xFFFF) - 1], display->file->header.header.magic == (('S' << 0) | ('P' << 8) | ('X' << 16) | ('G' << 24)), block);
	gtk_paned_set_end_child(GTK_PANED(display->display.root), child);
}
