#ifndef JKSSL_CMD_H
#define JKSSL_CMD_H

#define DECL_CMD(name) int cmd_##name(int argc, char **argv)

DECL_CMD(asn1parse);
DECL_CMD(base64);
DECL_CMD(bignum);
DECL_CMD(dgst);
DECL_CMD(dhparam);
DECL_CMD(dsa);
DECL_CMD(dsaparam);
DECL_CMD(ec);
DECL_CMD(ecparam);
DECL_CMD(enc);
DECL_CMD(gendsa);
DECL_CMD(genrsa);
DECL_CMD(help);
DECL_CMD(pkcs8);
DECL_CMD(rsa);
DECL_CMD(rsautl);
DECL_CMD(s_client);
DECL_CMD(speed);
DECL_CMD(version);
DECL_CMD(x509);

#undef DECL_CMD

void print_usage(void);

#endif
