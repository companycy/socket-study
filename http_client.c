#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#include "http_client.h"

// $ gcc -o http_client http_client.c
// $ ./http_client
// USAGE: http_client host page
// host: the website hostname. ex: 192.168.1.64
// page: the page to retrieve. ex: index.html

// ./http_client.exe 192.168.1.64 /cmp/data/login.login?username=123

void get_config_from_midware(const char *host, const char *page, char *buf, size_t len) {
  if (host == NULL || page == NULL) {
    perror("Could not connect");
    exit(1);
  }

  int sock = create_tcp_socket();
  char ip[16] = {0};
  get_ip(host, ip, sizeof(ip)-1);
  fprintf(stdout, "IP is %s\n", ip);

  struct sockaddr_in remote;
  remote.sin_family = AF_INET;
  int tmpres = inet_pton(AF_INET, ip, (void*)(&(remote.sin_addr.s_addr)));
  if (tmpres < 0) {
    perror("Can't set remote->sin_addr.s_addr");
    exit(1);
  } else if (tmpres == 0) {
    fprintf(stderr, "%s is not a valid IP address\n", ip);
    exit(1);
  }
  remote.sin_port = htons(80);	// http_port
  if (connect(sock, (const struct sockaddr*)&remote, sizeof(struct sockaddr)) < 0) {
    perror("Could not connect");
    exit(1);
  }

  char get_query[BUFSIZ] = {0};		// should be enough
  build_get_query(host, page, get_query, sizeof(get_query));
  fprintf(stderr, "Query <<START>>\n%sQuery <<END>>\n", get_query);

  int sent = 0;			// send the query to the server
  while (sent < strlen(get_query)) {
    tmpres = send(sock, get_query+sent, strlen(get_query)-sent, 0);
    if (tmpres == -1){
      perror("Can't send query");
      exit(1);
    }
    sent += tmpres;
  }

  int htmlstart = 0;
  char *htmlcontent, ret[BUFSIZ] = {0};
  while ((tmpres = recv(sock, ret, BUFSIZ, 0)) > 0) {
    if (htmlstart == 0) {
      /* Under certain conditions this will not work.
       * If the \r\n\r\n part is splitted into two messages
       * it will fail to detect the beginning of HTML content
       */
      htmlcontent = strstr(ret, "\r\n\r\n");
      if (htmlcontent != NULL){
	htmlstart = 1;
	htmlcontent += 4;
      }
    } else {
      htmlcontent = ret;
    }
    if (htmlstart) {
      printf("one more buf\n");
      fprintf(stdout, htmlcontent); // output
      strncpy(buf, htmlcontent, tmpres);
    }

    memset(ret, 0, tmpres);
  }
  if (tmpres < 0) {
    perror("Error receiving data");
  }

  close(sock);
}


int create_tcp_socket() {
  int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (sock < 0) {
    perror("Can't create TCP socket");
    exit(1);
  }
  return sock;
}

void get_ip(const char *host, char *ip, size_t iplen) {
  struct hostent *hent;
  if ((hent = gethostbyname(host)) == NULL) {
    perror("Can't get IP");
    exit(1);
  }
  if (inet_ntop(AF_INET, (void *)hent->h_addr_list[0], ip, iplen) == NULL) {
    perror("Can't resolve host");
    exit(1);
  }
}

// host: www.baidu.com
// page: /
// thus result looks like:
// GET / HTTP/1.0
// Host: www.baidu.com
// User-Agent: HTMLGET 1.0
void build_get_query(const char *host,
		     const char *page,
		     char *query, size_t len) {
  const char *tpl =
    "GET %s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n"; // 1.0 version
  // "GET /%s HTTP/1.1\r\nHost: %s\r\nUser-Agent: %s\r\nConnection: close\r\n\r\n";

  // -5 is to consider the %s %s %s in tpl and the ending \0
  // char *query =
  //   (char *)malloc(strlen(host)+strlen(page)+strlen(USERAGENT)+strlen(tpl)-5);
  sprintf(query, tpl, page, host, USERAGENT);
}

char *test_get_query() {
  const char *tpl = "GET /%s HTTP/1.0\r\nHost: %s\r\nUser-Agent: %s\r\n\r\n";
  const char *p = "";
  const char *h = "www.baidu.com";
  char *query = (char *)malloc(strlen(h)+strlen(p)+strlen(USERAGENT)+strlen(tpl)-5);
  sprintf(query, tpl, p, h, USERAGENT);
  return query;
}
