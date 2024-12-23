#include "performance.h"

#include "log.h"

#include <inttypes.h>
#include <string.h>
#include <limits.h>

struct performance_report g_performances[PERFORMANCE_LAST];

#ifdef WITH_PERFORMANCE
static const char *strings[PERFORMANCE_LAST] =
{
	"AABB_RENDER",
	"ADT_CULL",
	"ADT_OBJECTS_CULL",
	"COLLISIONS",
	"COLLISIONS_RENDER",
	"MCNK_CULL",
	"MCNK_RENDER",
	"MCNK_RENDER_BIND",
	"MCNK_RENDER_DATA",
	"MCNK_RENDER_DRAW",
	"MCLQ_CULL",
	"MCLQ_RENDER",
	"M2_BONES_RENDER",
	"M2_COLLISIONS_RENDER",
	"M2_CULL",
	"M2_LIGHTS_RENDER",
	"M2_PARTICLES_RENDER",
	"M2_RENDER",
	"M2_RENDER_BIND",
	"M2_RENDER_DATA",
	"M2_RENDER_DRAW",
	"M2_RIBBONS_RENDER",
	"SKYBOX_RENDER",
	"TAXI_RENDER",
	"WDL_CULL",
	"WDL_RENDER",
	"WMO_COLLISIONS_RENDER",
	"WMO_CULL",
	"WMO_LIGHTS_RENDER",
	"WMO_LIQUIDS_RENDER",
	"WMO_PORTALS_CULL",
	"WMO_PORTALS_RENDER",
	"WMO_RENDER",
	"WMO_RENDER_BIND",
	"WMO_RENDER_DATA",
	"WMO_RENDER_DRAW",
};
#endif

void performance_init(void)
{
#ifdef WITH_PERFORMANCE
	for (size_t i = 0; i < sizeof(g_performances) / sizeof(*g_performances); ++i)
	{
		struct performance_report *report = &g_performances[i];
		pthread_mutex_init(&report->mutex, NULL);
	}
#endif
}

void performance_reset(void)
{
#ifdef WITH_PERFORMANCE
	for (size_t i = 0; i < sizeof(g_performances) / sizeof(*g_performances); ++i)
	{
		struct performance_report *report = &g_performances[i];
		report->samples = 0;
		report->sum = 0;
		report->min = UINT_MAX;
		report->max = 0;
	}
#endif
}

void performance_dump(void)
{
#ifdef WITH_PERFORMANCE
	LOG_INFO("%21s | %7s | %6s | %6s | %8s | %7s", "category", "samples", "min", "max", "average", "total");
	LOG_INFO("----------------------+---------+--------+--------+----------+--------");
	for (size_t i = 0; i < sizeof(g_performances) / sizeof(*g_performances); ++i)
	{
		uint64_t samples = g_performances[i].samples;
		uint64_t min = samples ? g_performances[i].min / 1000 : 0;
		uint64_t max = samples ? g_performances[i].max / 1000 : 0;
		float avg = samples ? g_performances[i].samples : 1;
		avg = g_performances[i].sum / avg / 1000;
		uint64_t sum = samples ? g_performances[i].sum / 1000 : 0;
		LOG_INFO("%21s | %7" PRIu64 " | %6" PRIu64 " | %6" PRIu64 " | %8.2lf | %7" PRIu64, strings[i], samples, min, max, avg, sum);
	}
	LOG_INFO(" ");
#endif
}

void performance_add(enum performance_category category, uint64_t time)
{
#ifdef WITH_PERFORMANCE
	struct performance_report *report = &g_performances[category];
	pthread_mutex_lock(&report->mutex);
	report->samples++;
	report->sum += time;
	if (time < report->min)
		report->min = time;
	if (time > report->max)
		report->max = time;
	pthread_mutex_unlock(&report->mutex);
#else
	(void)category;
	(void)time;
#endif
}
