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
	
	rc = modbus_set_slave(ctxRTU, 6);
	rc = modbus_connect(ctxRTU);
	

	for (;;) {
		uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];

		rc = modbus_receive(ctxRTU, query);
		if (rc > 0) {
			modbus_reply(ctxRTU, query, rc, mb_mapping);
		}
		else if (rc == -1) {
			/* Connection closed by the client or error */
			printf("sykdebug: ERROR in modbus_receive(ctxRTU, query)\n");
			modbus_close(ctxRTU);
			modbus_free(ctxRTU);

			ctxRTU = modbus_new_rtu("/dev/ttyUSB0", 38400, 'E', 8, 1);
			modbus_set_debug(ctxRTU, TRUE);
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

int main()
{
	mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
		MODBUS_MAX_READ_REGISTERS, 0);
	if (mb_mapping == NULL) {
		fprintf(stderr, "Failed to allocate the mapping: %s\n",
			modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}
	threadModbusRTU((void *)mb_mapping);
}