#ifndef _HTTP_CLIENT_H_
#define _HTTP_CLIENT_H_


static const char USERAGENT[] = "HTMLGET 1.0";

int create_tcp_socket();
void get_ip(const char *host, char *ip, size_t iplen);
void build_get_query(const char *host, const char *page,
		     char *query, size_t len);
void usage();
void get_config_from_midware(const char *host, const char *page, char *buf, size_t len);

#endif  // _HTTP_CLIENT_H_
