#include "loader.h"

#include "gx/frame.h"

#include "map/map.h"

#include "obj/worldobj.h"
#include "obj/player.h"

#include "itf/interface.h"

#include "obj/unit.h"

#include "memory.h"
#include "camera.h"
#include "cache.h"
#include "log.h"
#include "wow.h"

#include <wow/mpq.h>

#include <gfx/window.h>

#include <jks/array.h>
#include <jks/list.h>

#include <sys/queue.h>

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>

#define ASYNC_THREADS 6
#define CULL_THREADS 6

#define INIT_MIN_DURATION 5000000
#define INIT_FRAME_DURATION 16000000 /* little bit less than frame for 60 fps */

MEMORY_DECL(GENERIC);

struct async_task
{
	enum async_task_type type;
	loader_load_fn_t fn;
	void *userdata;
	TAILQ_ENTRY(async_task) chain;
};

struct loader_object
{
	enum loader_object_type type;
	loader_init_fn_t init_fn;
	void *userdata;
	TAILQ_ENTRY(loader_object) chain;
};

struct async_worker
{
	struct wow_mpq_compound *mpq_compound;
	pthread_t thread;
};

struct loader
{
	pthread_mutex_t cull_mutex;
	pthread_mutex_t gc_mutex;
	struct jks_array workers; /* struct async_worker */
	TAILQ_HEAD(, async_task) tasks[ASYNC_TASK_LAST];
	pthread_mutex_t tasks_mutexes[ASYNC_TASK_LAST];
	struct jks_array cull_threads; /* pthread_t */
	pthread_cond_t cull_run_condition;
	pthread_cond_t cull_end_condition;
	TAILQ_HEAD(, loader_object) objects_to_init[LOADER_LAST];
	pthread_mutex_t objects_to_init_mutexes[LOADER_LAST];
	size_t cull_ended;
	size_t cull_ready;
	bool running;
};

static struct async_task *find_task(struct loader *loader)
{
	for (size_t i = 0; i < ASYNC_TASK_LAST; ++i)
	{
		pthread_mutex_lock(&loader->tasks_mutexes[i]);
		struct async_task *task = TAILQ_FIRST(&loader->tasks[i]);
		if (!task)
		{
			pthread_mutex_unlock(&loader->tasks_mutexes[i]);
			continue;
		}
		TAILQ_REMOVE(&loader->tasks[i], task, chain);
		pthread_mutex_unlock(&loader->tasks_mutexes[i]);
		return task;
	}
	return NULL;
}

static void *loader_run(void *data)
{
	struct loader *loader = data;
	size_t id = (size_t)-1;
	for (size_t i = 0; i < loader->workers.size; ++i)
	{
		if (pthread_equal(pthread_self(), JKS_ARRAY_GET(&loader->workers, i, struct async_worker)->thread))
		{
			id = i;
			break;
		}
	}
	if (id == (size_t)-1)
		return NULL;
	struct async_worker *worker = JKS_ARRAY_GET(&loader->workers, id, struct async_worker);
	worker->mpq_compound = wow_mpq_compound_new();
	if (!worker->mpq_compound)
	{
		LOG_ERROR("failed to create mpq compound");
		return NULL;
	}
	if (!wow_load_compound(g_wow, worker->mpq_compound))
	{
		LOG_ERROR("failed to load mpq compound");
		return NULL;
	}
	while (loader->running)
	{
		struct async_task *task = find_task(loader);
		if (task)
		{
			task->fn(worker->mpq_compound, task->userdata);
			mem_free(MEM_GENERIC, task);
		}
		else
		{
			usleep(5000);
		}
	}
	wow_mpq_compound_delete(worker->mpq_compound);
	return NULL;
}

static void cull_init(struct wow *wow)
{
	camera_handle_keyboard(wow->view_camera);
	camera_handle_mouse(wow->view_camera);
	gx_frame_clear_scene(wow->cull_frame);
	if (!camera_update_matrixes(wow->view_camera))
		LOG_ERROR("failed to update camera matrixes");
	gx_frame_copy_cameras(wow->cull_frame, wow->frustum_camera, wow->view_camera);
	if ((wow->wow_opt & WOW_OPT_RENDER_INTERFACE) && wow->interface)
		interface_update(wow->interface);
}

static int m2_instance_compare(const void *instance1, const void *instance2)
{
	float d1 = (*(const struct gx_m2_instance**)instance1)->frames[g_wow->cull_frame->id].distance_to_camera;
	float d2 = (*(const struct gx_m2_instance**)instance2)->frames[g_wow->cull_frame->id].distance_to_camera;
	if (d1 < d2)
		return 1;
	if (d1 > d2)
		return -1;
	return 0;
}

static void cull_fini(struct wow *wow)
{
	if (wow->cull_frame->render_lists.m2_transparent.entries.size)
		qsort(wow->cull_frame->render_lists.m2_transparent.entries.data, g_wow->cull_frame->render_lists.m2_transparent.entries.size, sizeof(struct gx_m2_instance*), m2_instance_compare);
	gx_frame_release_obj(wow->cull_frame);
	map_flag_clear(wow->map, MAP_FLAG_TILES_LOADED | MAP_FLAG_WDL_CULLED | MAP_FLAG_WMO_CULLED);
	JKS_HMAP_FOREACH(iter, g_wow->objects)
	{
		struct object *obj = *(struct object**)jks_hmap_iterator_get_value(&iter);
		if (object_is_unit(obj))
			((struct unit*)obj)->physics_ran = 0;
		if (object_is_worldobj(obj))
			((struct worldobj*)obj)->in_render_list = 0;
	}
}

static void cull_core(struct wow *wow)
{
	if (wow->map)
		map_cull(wow->map, wow->cull_frame);
	JKS_HMAP_FOREACH(iter, wow->objects)
	{
		struct object *obj = *(struct object**)jks_hmap_iterator_get_value(&iter);
		if (obj != (struct object*)g_wow->player && object_is_unit(obj))
			unit_physics((struct unit*)obj);
		if (object_is_worldobj(obj))
			worldobj_add_to_render((struct worldobj*)obj);
	}
}

static void *cull_run(void *data)
{
	struct loader *loader = data;
	pthread_mutex_lock(&loader->cull_mutex);
	while (loader->running)
	{
		if (!loader->cull_ready)
		{
			pthread_cond_wait(&loader->cull_run_condition, &loader->cull_mutex);
			continue;
		}
		if (loader->cull_ready == CULL_THREADS)
			cull_init(g_wow);
		loader->cull_ready--;
		pthread_mutex_unlock(&loader->cull_mutex);
		cull_core(g_wow);
		pthread_mutex_lock(&loader->cull_mutex);
		if (loader->cull_ended == CULL_THREADS - 1)
		{
			pthread_mutex_unlock(&loader->cull_mutex);
			cull_fini(g_wow);
			pthread_mutex_lock(&loader->cull_mutex);
		}
		loader->cull_ended++;
		pthread_cond_signal(&loader->cull_end_condition);
	}
	pthread_mutex_unlock(&loader->cull_mutex);
	return NULL;
}

void loader_start_cull(struct loader *loader)
{
	if (g_wow->wow_opt & WOW_OPT_ASYNC_CULL)
	{
		pthread_mutex_lock(&loader->cull_mutex);
		loader->cull_ended = 0;
		loader->cull_ready = CULL_THREADS;
		pthread_cond_broadcast(&loader->cull_run_condition);
		pthread_mutex_unlock(&loader->cull_mutex);
	}
	else
	{
		cull_init(g_wow);
	}
}

void loader_wait_cull(struct loader *loader)
{
	if (g_wow->wow_opt & WOW_OPT_ASYNC_CULL)
	{
		pthread_mutex_lock(&loader->cull_mutex);
		while (loader->cull_ended < CULL_THREADS)
			pthread_cond_wait(&loader->cull_end_condition, &loader->cull_mutex);
		pthread_mutex_unlock(&loader->cull_mutex);
	}
	else
	{
		cull_core(g_wow);
		cull_fini(g_wow);
	}
}

struct loader *loader_new(void)
{
	struct loader *loader = mem_malloc(MEM_GENERIC, sizeof(*loader));
	if (!loader)
		return NULL;
	loader->cull_ready = 0;
	loader->cull_ended = CULL_THREADS;
	loader->running = true;
	jks_array_init(&loader->cull_threads, sizeof(pthread_t), NULL, &jks_array_memory_fn_GENERIC);
	for (size_t i = 0; i < LOADER_LAST; ++i)
	{
		TAILQ_INIT(&loader->objects_to_init[i]);
		pthread_mutex_init(&loader->objects_to_init_mutexes[i], NULL);
	}
	for (size_t i = 0; i < ASYNC_TASK_LAST; ++i)
	{
		TAILQ_INIT(&loader->tasks[i]);
		pthread_mutex_init(&loader->tasks_mutexes[i], NULL);
	}
	jks_array_init(&loader->workers, sizeof(struct async_worker), NULL, &jks_array_memory_fn_GENERIC);
	if (!jks_array_resize(&loader->workers, ASYNC_THREADS))
	{
		LOG_ERROR("failed to resize workers array");
		goto err;
	}
	pthread_mutex_init(&loader->cull_mutex, NULL);
	pthread_mutex_init(&loader->gc_mutex, NULL);
	if (pthread_cond_init(&loader->cull_run_condition, NULL))
	{
		LOG_ERROR("failed to init pthread cull run condition");
		goto err;
	}
	if (pthread_cond_init(&loader->cull_end_condition, NULL))
	{
		LOG_ERROR("failed to init pthread cull end condition");
		goto err;
	}
	for (size_t i = 0; i < loader->workers.size; ++i)
	{
		char title[256];
		snprintf(title, sizeof(title), "WoW loader %d", (int)i);
		struct async_worker *worker = JKS_ARRAY_GET(&loader->workers, i, struct async_worker);
		if (pthread_create(&worker->thread, NULL, loader_run, loader))
		{
			LOG_ERROR("failed to create worker pthread");
			goto err;
		}
#ifndef _WIN32
		pthread_setschedprio(worker->thread, 1);
#endif
	}
	if (!jks_array_reserve(&loader->cull_threads, CULL_THREADS))
	{
		LOG_ERROR("failed to resize cull threads");
		goto err;
	}
	for (size_t i = 0; i < CULL_THREADS; ++i)
	{
		pthread_t *cull_thread = JKS_ARRAY_GET(&loader->cull_threads, i, pthread_t);
		if (pthread_create(cull_thread, NULL, cull_run, loader))
		{
			LOG_ERROR("failed to create cull pthread");
			goto err;
		}
#ifndef _WIN32
		pthread_setschedprio(*cull_thread, 0);
#endif
	}
	return loader;

err:
	mem_free(MEM_GENERIC, loader);
	return NULL;
}

void loader_delete(struct loader *loader)
{
	if (!loader)
		return;
	loader->running = false;
	for (size_t i = 0; i < loader->workers.size; ++i)
	{
		struct async_worker *worker = JKS_ARRAY_GET(&loader->workers, i, struct async_worker);
		pthread_join(worker->thread, NULL);
	}
	jks_array_destroy(&loader->workers);
	pthread_cond_broadcast(&loader->cull_run_condition);
	for (size_t i = 0; i < loader->cull_threads.size; ++i)
	{
		pthread_t *thread = JKS_ARRAY_GET(&loader->cull_threads, i, pthread_t);
		pthread_join(*thread, NULL);
	}
	jks_array_destroy(&loader->cull_threads);
	pthread_mutex_destroy(&loader->cull_mutex);
	pthread_mutex_destroy(&loader->gc_mutex);
	pthread_cond_destroy(&loader->cull_run_condition);
	pthread_cond_destroy(&loader->cull_end_condition);
	for (size_t i = 0; i < LOADER_LAST; ++i)
	{
		struct loader_object *object;
		while ((object = TAILQ_FIRST(&loader->objects_to_init[i])))
		{
			TAILQ_REMOVE(&loader->objects_to_init[i], object, chain);
			mem_free(MEM_GENERIC, object);
		}
		pthread_mutex_destroy(&loader->objects_to_init_mutexes[i]);
	}
	for (size_t i = 0; i < ASYNC_TASK_LAST; ++i)
	{
		struct async_task *task;
		while ((task = TAILQ_FIRST(&loader->tasks[i])))
		{
			TAILQ_REMOVE(&loader->tasks[i], task, chain);
			mem_free(MEM_GENERIC, task);
		}
		pthread_mutex_destroy(&loader->tasks_mutexes[i]);
	}
	mem_free(MEM_GENERIC, loader);
}

bool loader_has_async(struct loader *loader)
{
	for (size_t i = 0; i < ASYNC_TASK_LAST; ++i)
	{
		if (!TAILQ_EMPTY(&loader->tasks[i]))
			return true;
	}
	return false;
}

bool loader_has_loading(struct loader *loader)
{
	for (size_t i = 0; i < LOADER_LAST; ++i)
	{
		if (!TAILQ_EMPTY(&loader->objects_to_init[i]))
			return true;
	}
	return false;
}

static void do_init(struct loader *loader)
{
	uint64_t started = nanotime();
	uint64_t tmp = started;
	for (size_t i = 0; i < LOADER_LAST; ++i)
	{
		pthread_mutex_lock(&loader->objects_to_init_mutexes[i]);
		struct loader_object *object;
		while ((object = TAILQ_FIRST(&loader->objects_to_init[i])))
		{
			pthread_mutex_unlock(&loader->objects_to_init_mutexes[i]);
			if (object->init_fn(object->userdata))
			{
				pthread_mutex_lock(&loader->objects_to_init_mutexes[i]);
				TAILQ_REMOVE(&loader->objects_to_init[i], object, chain);
				pthread_mutex_unlock(&loader->objects_to_init_mutexes[i]);
			}
			tmp = nanotime();
			if (tmp - started < INIT_MIN_DURATION)
				continue;
			if (tmp - g_wow->frametime > INIT_FRAME_DURATION)
			{
				pthread_mutex_unlock(&loader->objects_to_init_mutexes[i]);
				return;
			}
		}
		pthread_mutex_unlock(&loader->objects_to_init_mutexes[i]);
	}
}

void loader_push(struct loader *loader, enum async_task_type type, loader_load_fn_t fn, void *data)
{
	assert(loader->running);
	struct async_task *task = mem_malloc(MEM_GENERIC, sizeof(*task));
	if (!task)
	{
		LOG_ERROR("failed to add task to queue");
		abort();
	}
	task->type = type;
	task->fn = fn;
	task->userdata = data;
	pthread_mutex_lock(&loader->tasks_mutexes[type]);
	TAILQ_INSERT_TAIL(&loader->tasks[type], task, chain);
	pthread_mutex_unlock(&loader->tasks_mutexes[type]);
}

void loader_tick(struct loader *loader)
{
	do_init(loader);
}

void loader_init_object(struct loader *loader, enum loader_object_type type, loader_init_fn_t fn, void *userdata)
{
	struct loader_object *object = mem_malloc(MEM_GENERIC, sizeof(*object));
	if (!object)
	{
		LOG_ERROR("failed to add object to init buffer");
		abort();
	}
	object->type = type;
	object->init_fn = fn;
	object->userdata = userdata;
	pthread_mutex_lock(&loader->objects_to_init_mutexes[type]);
	TAILQ_INSERT_TAIL(&loader->objects_to_init[type], object, chain);
	pthread_mutex_unlock(&loader->objects_to_init_mutexes[type]);
}
