#define __STRICT_ANSI__
#define _ISOC99_SOURCE
#define _POSIX_C_SOURCE 200112L

#undef NDEBUG

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

int set_up_timeout(struct timeval *timeout, int seconds);
int connect_to(const char *remote_addr, const char *remote_port);
int connect_by_addrinfo(struct addrinfo *address_info);
int raise_socket_flags(int handle_socket, long new_socket_flags);
int wait_for_socket(int handle_socket, struct timeval *timeout);
int get_socket_error(int handle_socket);

int main(int argc, char **argv) {
	int handle_socket;
	struct timeval timeout;

	if (argc != (ARGNO_TOTAL + 1)) {
		fprintf(stderr,
				"Exactly three arguments expected: hostname, numeric port, timeout in seconds. %d received instead.\n",
				argc - 1);

		exit(EXIT_FAILURE);
	}

	if (! set_up_timeout(&timeout, atoi(argv[ARGNO_TIMEOUT]))) {
		fprintf(stderr,
				"Invalid timeout \"%s\" specified. Must be a positive integer.\n",
				argv[ARGNO_TIMEOUT]);

		exit(EXIT_FAILURE);
	}

	if ((handle_socket = connect_to(argv[ARGNO_REMOTE_ADDR], argv[ARGNO_REMOTE_PORT])) == -1) {
		fprintf(stderr,
				"Failed to open a socket. Invalid hostname or port number?\n");

		exit(EXIT_FAILURE);
	}

	if (wait_for_socket(handle_socket, &timeout) <= 0) {
		fprintf(stderr, "Connection timed out.\n");

		exit(EXIT_FAILURE);
	}

	if (get_socket_error(handle_socket) != 0) {
		fprintf(stderr, "Connection closed.\n");

		exit(EXIT_FAILURE);
	}

	exit(EXIT_SUCCESS);
}

int set_up_timeout(struct timeval *timeout, int seconds) {
	if (seconds <= 0) {
		return 0;
	}

	timeout->tv_usec = 0;
	timeout->tv_sec = seconds;

	return seconds;
}

int connect_to(const char *remote_addr, const char *remote_port) {
	struct addrinfo *address_info;
	int handle_socket;

	if (getaddrinfo(remote_addr, remote_port, NULL, &address_info) != 0) {
		return -1;
	}

	handle_socket = connect_by_addrinfo(address_info);

	freeaddrinfo(address_info);

	return handle_socket;
}

int connect_by_addrinfo(struct addrinfo *address_info) {
	int handle_socket;

	if ((handle_socket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
		return handle_socket;
	}

	if (raise_socket_flags(handle_socket, O_NONBLOCK) == -1) {
		return -1;
	}

	if (connect(handle_socket, address_info->ai_addr, address_info->ai_addrlen) != -1) {
		return -1;
	}

	if (errno != EINPROGRESS) {
		return -1;
	}

	return handle_socket;	
}

int raise_socket_flags(int handle_socket, long new_socket_flags) {
	long socket_flags;

	if ((socket_flags = fcntl(handle_socket, F_GETFL, NULL)) == -1) {
		return socket_flags;
	}

	socket_flags |= new_socket_flags;

	return fcntl(handle_socket, F_SETFL, socket_flags);
}

int wait_for_socket(int handle_socket, struct timeval *timeout) {
	fd_set handles_monitored;

	FD_ZERO(&handles_monitored);
	FD_SET(handle_socket, &handles_monitored);

	return select(handle_socket + 1, NULL, &handles_monitored, NULL, timeout);
}

int get_socket_error(int handle_socket) {
	int socket_option;
	socklen_t socket_option_length = sizeof(socket_option);

	if (getsockopt(handle_socket, SOL_SOCKET, SO_ERROR, (void *)&socket_option, &socket_option_length) != 0) {
		return -1;
	}

	return socket_option;
}

