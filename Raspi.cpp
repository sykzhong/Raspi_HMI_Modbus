/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
	Used on Raspeberry Pi, as a slave to receive the indication from HMI. 
	At same time, it has the ability to read or write the register on HMI.
	Use TCP/IP to connect with the HMI
*/
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>

#include <modbus/modbus.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace std;

#define NB_CONNECTION    5
static modbus_t *ctx = NULL;
static modbus_mapping_t *mb_mapping;
static int server_socket = -1;	

#include <modbus/modbus.h>

static void close_sigint(int dummy)
{
	
    if (server_socket != -1) {
        close(server_socket);
    }
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}

int main()
{
	uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
    int 	master_socket;
    int 	rc;
    fd_set 	refset;
    fd_set 	rdset;

    /* Maximum file descriptor number */
    int 	fdmax;

    int     sykdebugnb = 0;

    ctx = modbus_new_tcp("192.168.142.129", 502);

    /*sykfix: modbus_mapping_new should give rasp pi write bits and write register*/
    mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);

    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    server_socket = modbus_tcp_listen(ctx, NB_CONNECTION);
    if (server_socket == -1) {
        fprintf(stderr, "Unable to listen TCP connection\n");
        modbus_free(ctx);
        return -1;
    }

    signal(SIGINT, close_sigint);		//InterruptKey to close the signal

        /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    for(;;)
    {
    	rdset = refset;
    	/*sykfix: need to add write fdset. And noticed the select function is run by blocked*/
    	if(select(fdmax+1, &rdset, NULL, NULL, NULL) == -1)
    	{
    		perror("Server select() failure.");
    		close_sigint(1);
    	}

    	/* Run through the existing connections looking for data to be
         * read */
    	for(master_socket = 0; master_socket <= fdmax; master_socket++)
    	{
    		if(!FD_ISSET(master_socket, &rdset))
    		{
    			continue;
    		}

    		if(master_socket == server_socket)
    		{
    			/*a socket client is asking a new connection here?*/
    			socklen_t addrlen;
    			struct sockaddr_in clientaddr;
    			int newfd;

    			/*Handle new connections*/
    			addrlen = sizeof(clientaddr);
    			memset(&clientaddr, 0, sizeof(clientaddr));
    			newfd = accept(server_socket, (struct sockaddr*)&clientaddr, &addrlen);

    			if(newfd == -1)
    			{
    				perror("Server accept() error");
       			}
       			else
       			{
       				FD_SET(newfd, &refset);
       				if(newfd > fdmax)
       				{
       					/*Keep track of the maximum of socket*/
       					fdmax = newfd;
       				}
       				printf("New connection from %s:%d on socket %d\n",
       					inet_ntoa(clientaddr.sin_addr), clientaddr.sin_port, newfd);
       			}
    		}
    		else
    		{
    			modbus_set_socket(ctx, master_socket);
    			rc = modbus_receive(ctx, query);
    			if(rc > 0)
    			{
    				modbus_reply(ctx, query, rc, mb_mapping);
                    sykdebugnb++;
                    printf("sykdebugnb = %d\n\r", sykdebugnb);
    			}
    			else if(rc == -1)
    			{
    				/* This example server in ended on connection closing or
                     * any errors. */
                    printf("Connection closed on socket %d\n", master_socket);
                    close(master_socket);

                    /* Remove from reference set */
                    FD_CLR(master_socket, &refset);

                    if (master_socket == fdmax)
                        fdmax--;
    			}
                if(sykdebugnb == 2)
                {
                    printf("sykdebug: begin to read\n");
                    uint16_t *axis_one_reply_register = (uint16_t *)malloc(sizeof(uint16_t));
                    memset(axis_one_reply_register, 0, sizeof(uint16_t));
                    rc = modbus_read_registers(ctx, 20, 1, axis_one_reply_register);
                    printf("Address = %d, value = %d\n", 1, axis_one_reply_register[0]);
                }
    		}
    	}
    }

}
