#ifndef ARPA_NAMESER_H
#define ARPA_NAMESER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ns_type
{
	ns_t_a          = 1,
	ns_t_ns         = 2,
	ns_t_md         = 3,
	ns_t_mf         = 4,
	ns_t_cname      = 5,
	ns_t_soa        = 6,
	ns_t_mb         = 7,
	ns_t_mg         = 8,
	ns_t_mr         = 9,
	ns_t_null       = 10,
	ns_t_wks        = 11,
	ns_t_ptr        = 12,
	ns_t_hinfo      = 13,
	ns_t_minfo      = 14,
	ns_t_mx         = 15,
	ns_t_txt        = 16,
	ns_t_rp         = 17,
	ns_t_afsdb      = 18,
	ns_t_x25        = 19,
	ns_t_isdn       = 20,
	ns_t_rt         = 21,
	ns_t_nsap       = 22,
	ns_t_nsap_ptr   = 23,
	ns_t_sig        = 24,
	ns_t_key        = 25,
	ns_t_px         = 26,
	ns_t_gpos       = 27,
	ns_t_aaaa       = 28,
	ns_t_loc        = 29,
	ns_t_nxt        = 30,
	ns_t_eid        = 31,
	ns_t_nb         = 32,
	ns_t_srv        = 33,
	ns_t_atma       = 34,
	ns_t_naptr      = 35,
	ns_t_kx         = 36,
	ns_t_cert       = 37,
	ns_t_a6         = 38,
	ns_t_dname      = 39,
	ns_t_sink       = 40,
	ns_t_opt        = 41,
	ns_t_apl        = 42,
	ns_t_ds         = 43,
	ns_t_sshfp      = 44,
	ns_t_ipseckey   = 45,
	ns_t_rrsig      = 46,
	ns_t_nsec       = 47,
	ns_t_dnskey     = 48,
	ns_t_dhcid      = 49,
	ns_t_nsec3      = 50,
	ns_t_nsec3param = 51,
	ns_t_tlsa       = 52,
	ns_t_smimea     = 53,
	ns_t_hip        = 55,
	ns_t_ninfo      = 56,
	ns_t_rkey       = 57,
	ns_t_talink     = 58,
	ns_t_cds        = 59,
	ns_t_cdnskey    = 60,
	ns_t_openpgpkey = 61,
	ns_t_csync      = 62,
	ns_t_zonemd     = 63,
	ns_t_svcb       = 64,
	ns_t_https      = 65,
	ns_t_spf        = 99,
	ns_t_uinfo      = 100,
	ns_t_uid        = 101,
	ns_t_gid        = 102,
	ns_t_unspec     = 103,
	ns_t_nid        = 104,
	ns_t_l32        = 105,
	ns_t_l64        = 106,
	ns_t_lp         = 107,
	ns_t_eui48      = 108,
	ns_t_eui64      = 109,
	ns_t_tkey       = 249,
	ns_t_tsig       = 250,
	ns_t_ixfr       = 251,
	ns_t_axfr       = 252,
	ns_t_mailb      = 253,
	ns_t_maila      = 254,
	ns_t_uri        = 256,
	ns_t_caa        = 257,
	ns_t_avc        = 258,
	ns_t_doa        = 259,
	ns_t_ta         = 32768,
	ns_t_dlv        = 32769,
} ns_type;

typedef enum ns_opcode
{
	ns_o_query  = 0,
	ns_o_iquery = 1,
	ns_o_status = 2,
	ns_o_notify = 4,
	ns_o_update = 5,
} ns_opcode;

typedef enum ns_class
{
	ns_c_in = 1,
	ns_c_cs = 2,
	ns_c_ch = 3,
	ns_c_hs = 4,
} ns_class;

typedef enum ns_sect
{
	ns_s_qd = 0,
	ns_s_an = 1,
	ns_s_ns = 2,
	ns_s_ar = 3,
	ns_s_max = 4,
} ns_sect;

typedef enum ns_flag
{
	ns_f_qr,
	ns_f_opcode,
	ns_f_aa,
	ns_f_tc,
	ns_f_rd,
	ns_f_ra,
	ns_f_z,
	ns_f_rcode,
} ns_flag;

typedef struct ns_msg
{
	const uint8_t *msg;
	const uint8_t *eom;
	uint16_t ident;
	uint16_t flags;
	uint16_t counts[4];
} ns_msg;

typedef struct ns_rr
{
	char name[256];
	uint16_t type;
	uint16_t class;
	uint32_t ttl;
	uint16_t rdlen;
	const uint8_t *rdata;
} ns_rr;

#define ns_msg_id(msg) ((msg).ident)
#define ns_msg_count(msg, sec) ((msg).counts[sec])

#define ns_rr_name(rr) ((rr).name)
#define ns_rr_type(rr) ((rr).type)
#define ns_rr_class(rr) ((rr).class)
#define ns_rr_ttl(rr) ((rr).ttl)
#define ns_rr_rdlen(rr) ((rr).rdlen)
#define ns_rr_rdata(rr) ((rr).rdata)

#define QUERY ns_o_query
#define IQUERY ns_o_iquery
#define STATUS ns_o_status
#define NOTIFY ns_o_notify
#define UPDATE ns_o_update

int ns_initparse(const uint8_t *data, int len, ns_msg *msg);
int ns_msg_getflag(ns_msg msg, enum ns_flag flag);
int ns_parserr(ns_msg *msg, ns_sect sect, int id, ns_rr *rr);

#ifdef __cplusplus
}
#endif

#endif
