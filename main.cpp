/*
* Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
*
* SPDX-License-Identifier: BSD-3-Clause
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include <modbus/modbus.h>

#if defined(_WIN32)
#include <ws2tcpip.h>
#else
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define NB_CONNECTION    5

static modbus_t *ctx = NULL;
static modbus_mapping_t *mb_mapping;

static int server_socket = -1;

static void close_sigint(int dummy)
{
	if (server_socket != -1) {
		close(server_socket);
	}
	modbus_free(ctx);
	modbus_mapping_free(mb_mapping);

	exit(dummy);
}

void * threadModbusRTU(void *arg)
{
	modbus_t * ctxRTU = NULL;
	modbus_mapping_t *mb_mapping = (modbus_mapping_t *)arg;
	int rc;
	printf("sykdebug: new thread\n");

	ctxRTU = modbus_new_rtu("/dev/ttyUSB0", 38400, 'E', 8, 1);
	modbus_set_debug(ctxRTU, TRUE);

	rc = modbus_rtu_set_serial_mode(ctxRTU, MODBUS_RTU_RS485);
	
	rc = modbus_set_slave(ctxRTU, 5);
	rc = modbus_connect(ctxRTU);
	

	for (;;) {
		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

		rc = modbus_receive(ctxRTU, query);
		if (rc > 0) {
			modbus_reply(ctxRTU, query, rc, mb_mapping);
		}
		else if (rc == -1) {
			/* Connection closed by the client or error */
			modbus_close(ctxRTU);
			modbus_free(ctxRTU);

			ctxRTU = modbus_new_rtu("/dev/ttyUSB0", 38400, 'E', 8, 1);
			rc = modbus_rtu_set_serial_mode(ctxRTU, MODBUS_RTU_RS232);
			rc = modbus_set_slave(ctxRTU, 6);
			rc = modbus_connect(ctxRTU);

			//break;
		}
	}

	printf("Quit the loop: %s\n", modbus_strerror(errno));

	/* For RTU, skipped by TCP (no TCP connect) */
	modbus_close(ctxRTU);
	modbus_free(ctxRTU);

	return 0;
}

int main(void)
{
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
	int master_socket;
	int rc;
	fd_set refset;
	fd_set rdset;
	/* Maximum file descriptor number */
	int fdmax;

	ctx = modbus_new_tcp("127.0.0.1", 502);

	mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
		MODBUS_MAX_READ_REGISTERS, 0);
	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
			modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	pthread_t th;
	int ret;
	int *thread_ret = NULL;
	ret = pthread_create(&th, NULL, threadModbusRTU, mb_mapping);
	if (ret != 0) {
		printf("Create thread error!\n");
		return -1;
	}

	server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);
	if (server_socket == -1) {
		fprintf(stderr, "Unable to listen TCP connection\n");
		modbus_free(ctx);
		return -1;
	}

	signal(SIGINT, close_sigint);

	/* Clear the reference set of socket */
	FD_ZERO(&refset);
	/* Add the server socket */
	FD_SET(server_socket, &refset);

	/* Keep track of the max file descriptor */
	fdmax = server_socket;

	for (;;) {
		rdset = refset;
		if (select(fdmax + 1, &rdset, NULL, NULL, NULL) == -1) {
			perror("Server select() failure.");
			close_sigint(1);
		}

		/* Run through the existing connections looking for data to be
		* read */
		for (master_socket = 0; master_socket <= fdmax; master_socket++) {

			if (!FD_ISSET(master_socket, &rdset)) {
				continue;
			}

			if (master_socket == server_socket) {
				/* A client is asking a new connection */
				socklen_t addrlen;
				struct sockaddr_in clientaddr;
				int newfd;

				/* Handle new connections */
				addrlen = sizeof(clientaddr);
				memset(&clientaddr, 0, sizeof(clientaddr));
				newfd = accept(server_socket, (struct sockaddr *)&clientaddr, &addrlen);
				if (newfd == -1) {
					perror("Server accept() error");
				}
				else {
					FD_SET(newfd, &refset);

					if (newfd > fdmax) {
						/* Keep track of the maximum */
						fdmax = newfd;
					}
					printf("New connection from %s:%d on socket %d\n",
						inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
				}
			}
			else {
				modbus_set_socket(ctx, master_socket);
				rc = modbus_receive(ctx, query);
				if (rc > 0) {
					modbus_reply(ctx, query, rc, mb_mapping);
				}
				else if (rc == -1) {
					/* This example server in ended on connection closing or
					* any errors. */
					printf("Connection closed on socket %d\n", master_socket);
					close(master_socket);

					/* Remove from reference set */
					FD_CLR(master_socket, &refset);

					if (master_socket == fdmax) {
						fdmax--;
					}
				}
			}
		}
	}

	return 0;
}
