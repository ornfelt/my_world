#include "nmap.h"

#include <arpa/inet.h>

#include <netdb.h>
#include <stdio.h>

static int port_status_opened(struct env *env, struct port_result *result)
{
	if (env->scans & SCAN_SYN)
	{
		if (result->status_syn == OPEN)
			return 1;
	}
	if (env->scans & SCAN_WIN)
	{
		if (result->status_win == OPEN)
			return 1;
	}
	return 0;
}

static char *get_scan_conclusion(struct env *env, struct port_result *result)
{
	if (env->scans & SCAN_SYN)
	{
		if (result->status_syn == OPEN)
			return "Open";
		if (result->status_syn == FILTERED)
			return "Filtered";
		return "Closed";
	}
	if (env->scans & SCAN_WIN)
	{
		if (result->status_win == OPEN)
			return "Open";
		if (result->status_win == CLOSED)
			return "Closed";
		return "Filtered";
	}
	if (env->scans & SCAN_ACK)
	{
		if (result->status_ack == FILTERED)
			return "Filtered";
		return "Unfiltered";
	}
	if (env->scans & SCAN_XMAS)
	{
		if (result->status_xmas == OPEN_FILTERED)
			return "Filtered";
	}
	if (env->scans & SCAN_NULL)
	{
		if (result->status_null == OPEN_FILTERED)
			return "Filtered";
	}
	if (env->scans & SCAN_FIN)
	{
		if (result->status_fin == OPEN_FILTERED)
			return "Filtered";
	}
	if (env->scans & SCAN_UDP)
	{
		if (result->status_udp == OPEN_FILTERED)
			return "Filtered";
	}
	if (env->scans & SCAN_MAIM)
	{
		if (result->status_maim == OPEN_FILTERED)
			return "Filtered";
	}
	return "Closed";
}

static void get_scan_result_str(char *dst, size_t size, const char *type,
                                enum port_status result)
{
	char *status;
	switch (result)
	{
		case OPEN:
			status = "(Open)";
			break;
		case FILTERED:
			status = "(Filtered)";
			break;
		case CLOSED:
			status = "(Closed)";
			break;
		case UNFILTERED:
			status = "(Unfiltered)";
			break;
		case OPEN_FILTERED:
			status = "(Open|Filtered)";
			break;
		default:
			status = "";
			break;
	}
	snprintf(dst, size, "%s%s", type, status);
}

static char *get_service_name(uint16_t port)
{
	struct servent *result;

	result = getservbyport(htons(port), NULL);
	if (!result)
		return "unassigned";
	return result->s_name;
}

static void print_result_port_mult_part(const char *type, enum port_status status,
                                        int i)
{
	char tmp[64];

	get_scan_result_str(tmp, sizeof(tmp), type, status);
	if (i != 1)
		printf("\n%-5s %-19s ", "", "");
	printf("%-20s ", tmp);
}

static void print_result_port_mult(struct env *env, struct port_result *result)
{
	int i = 0;
	if (env->scans & SCAN_SYN)
		print_result_port_mult_part("SYN", result->status_syn, ++i);
	if (env->scans & SCAN_NULL)
		print_result_port_mult_part("NULL", result->status_null, ++i);
	if (env->scans & SCAN_ACK)
		print_result_port_mult_part("ACK", result->status_ack, ++i);
	if (env->scans & SCAN_FIN)
		print_result_port_mult_part("FIN", result->status_fin, ++i);
	if (env->scans & SCAN_XMAS)
		print_result_port_mult_part("XMAS", result->status_xmas, ++i);
	if (env->scans & SCAN_UDP)
		print_result_port_mult_part("UDP", result->status_udp, ++i);
	if (env->scans & SCAN_WIN)
		print_result_port_mult_part("WIN", result->status_win, ++i);
	if (env->scans & SCAN_MAIM)
		print_result_port_mult_part("MAIM", result->status_maim, ++i);
	printf("%-20s\n\n", get_scan_conclusion(env, result));
}

static void print_result_port(struct env *env, struct port_result *result,
                              uint16_t port)
{
	char tmp[64];
	printf("%-5d %-19s ", port, get_service_name(port));
	if (env->scans_count != 1)
	{
		print_result_port_mult(env, result);
		return;
	}
	if (env->scans & SCAN_SYN)
	{
		get_scan_result_str(tmp, sizeof(tmp), "SYN",
		                    result->status_syn);
	}
	else if (env->scans & SCAN_NULL)
	{
		get_scan_result_str(tmp, sizeof(tmp), "NULL",
		                    result->status_null);
	}
	else if (env->scans & SCAN_ACK)
	{
		get_scan_result_str(tmp, sizeof(tmp), "ACK",
		                    result->status_ack);
	}
	else if (env->scans & SCAN_FIN)
	{
		get_scan_result_str(tmp, sizeof(tmp), "FIN",
		                    result->status_fin);
	}
	else if (env->scans & SCAN_XMAS)
	{
		get_scan_result_str(tmp, sizeof(tmp), "XMAS",
		                    result->status_xmas);
	}
	else if (env->scans & SCAN_UDP)
	{
		get_scan_result_str(tmp, sizeof(tmp), "UDP",
		                    result->status_udp);
	}
	else if (env->scans & SCAN_WIN)
	{
		get_scan_result_str(tmp, sizeof(tmp), "WIN",
		                    result->status_win);
	}
	else if (env->scans & SCAN_MAIM)
	{
		get_scan_result_str(tmp, sizeof(tmp), "MAIM",
		                    result->status_maim);
	}
	printf("%-20s %-20s\n", tmp, get_scan_conclusion(env, result));
}

void print_result(struct env *env, struct host *host)
{
	printf("\n");
	printf("Open ports:\n");
	printf("%-5s %-19s %-20s %-20s\n",
	       "Port", "Service Name", "Results", "Conclusion");
	for (uint32_t i = 0; i < 65536; ++i)
	{
		if (!env->ports[i])
			continue;
		if (port_status_opened(env, &host->results[i]))
			print_result_port(env, &host->results[i], i);
	}
	printf("\n");
	printf("Filtered/Unfiltered/Closed ports:\n");
	printf("%-5s %-19s %-20s %-20s\n",
	       "Port", "Service Name", "Results", "Conclusion");
	for (uint32_t i = 0; i < 65536; ++i)
	{
		if (!env->ports[i])
			continue;
		if (!port_status_opened(env, &host->results[i]))
			print_result_port(env, &host->results[i], i);
	}
}
