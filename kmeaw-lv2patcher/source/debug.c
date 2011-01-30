#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include "debug.h"

ssize_t debug_print_to_socket(int sockd, const void *vptr)
{
	size_t n = strlen(vptr);
	size_t nleft;
	ssize_t nwritten;
	const char *buffer;
	buffer = vptr;
	nleft = n;

	while (nleft > 0) {
		if ((nwritten = send(sockd, buffer, nleft, 0)) <= 0) {
			if (errno == EINTR)
				nwritten = 0;
			else
				return -1;
		}
		nleft -= nwritten;
		buffer += nwritten;
	}

	return n;
}

void debug_wait_for_client()
{
#if DEBUG
	int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(DEBUG_PORT);
	bind(listen_sock, (struct sockaddr *)&servaddr, sizeof(servaddr));
	listen(listen_sock, 1024);
	debug_sock = accept(listen_sock, NULL, NULL);
#endif
}
