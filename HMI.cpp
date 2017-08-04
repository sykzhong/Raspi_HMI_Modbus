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

#include <modbus/modbus.h>

#define LOOP				1
#define SERVER_ID			6
#define ADDRESS_AXIS_ONE	20

static modbus_t* ctx;

int main()
{
	int rc;
	uint16_t addr = ADDRESS_AXIS_ONE;
	int nb_fail = 0;


	// ctx = modbus_new_tcp("192.168.123.156", 502);		//Connect to windows
	ctx = modbus_new_tcp("192.168.142.129", 502);
	modbus_set_debug(ctx, TRUE);
	modbus_set_slave(ctx, 1);		//If server use slave ID, then there need to set too

	if(modbus_connect(ctx) == -1)
	{
		fprintf(stderr, "Coneection failed: %s\n", modbus_strerror(errno));
		modbus_free(ctx);
		return -1;
	}

	uint16_t *axis_one_register = (uint16_t *)malloc(sizeof(uint16_t));
	memset(axis_one_register, 0, sizeof(uint16_t));

	uint16_t *axis_one_reply_register = (uint16_t *)malloc(sizeof(uint16_t));
	memset(axis_one_reply_register, 0, sizeof(uint16_t));

	/*assigment of axis one's position*/
	*axis_one_register = 256;

	/*send register info and get the reply*/
	rc = modbus_write_register(ctx, addr, *axis_one_register);
	if(rc != 1)
	{
		printf("ERROR modbus_write_bit (%d)\n", rc);
        printf("Address = %d, value = %d\n", addr, axis_one_register[0]);
        nb_fail++;
	}
	else
	{
		rc = modbus_read_registers(ctx, addr, 1, axis_one_reply_register);
		if(*axis_one_register != *axis_one_reply_register)
		{
			printf("ERROR modbus_read_register\n");
            nb_fail++;
		}
		else
		{
			printf("SUCCESS modbus_read_register\n");
		}
		printf("Address = %d, value %d (0x%X) and %d (0x%X)\n",
			addr, *axis_one_register, *axis_one_register,
			*axis_one_reply_register, *axis_one_reply_register);
	}
	
	/*Get the result*/
	printf("Test: ");		
    if (nb_fail)
        printf("%d FAILS\n", nb_fail);
    else
        printf("SUCCESS\n");

    /*Free the memory*/
    free(axis_one_register);
    free(axis_one_reply_register);

    modbus_close(ctx);
    modbus_free(ctx);

}