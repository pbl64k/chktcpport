
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>

#define ARGNO_REMOTE_ADDR 1
#define ARGNO_REMOTE_PORT 2
#define ARGNO_TIMEOUT 3

#define ARGNO_TOTAL 3

int main(int argc, char **argv) {
	int handle_socket;
	int socket_option;
	socklen_t socket_option_length = sizeof(socket_option);
	long socket_flags;
	struct addrinfo *address_info;
	struct timeval timeout;
	fd_set handles_monitored;

	assert(argc == (ARGNO_TOTAL + 1));

	timeout.tv_usec = 0;
	assert((timeout.tv_sec = atoi(argv[ARGNO_TIMEOUT])) > 0);

	if (getaddrinfo(argv[ARGNO_REMOTE_ADDR], argv[ARGNO_REMOTE_PORT], NULL, &address_info) != 0) {
		exit(EXIT_FAILURE);
	}

	assert((handle_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) != -1);

	assert((socket_flags = fcntl(handle_socket, F_GETFL, NULL)) != -1);
	socket_flags |= O_NONBLOCK;
	assert(fcntl(handle_socket, F_SETFL, socket_flags) != -1);

	assert(connect(handle_socket, address_info->ai_addr, address_info->ai_addrlen) == -1);

	assert(errno == EINPROGRESS);

	FD_ZERO(&handles_monitored);
	FD_SET(handle_socket, &handles_monitored);

	if (select(handle_socket + 1, NULL, &handles_monitored, NULL, &timeout) < 0)
	{
		exit(EXIT_FAILURE);
	}

	assert(getsockopt(handle_socket, SOL_SOCKET, SO_ERROR, (void *)&socket_option, &socket_option_length) == 0);

	if (socket_option)
	{
		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}
