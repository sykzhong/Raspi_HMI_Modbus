#include "Raspi.h"

modbus_t* RaspiServer::ctx = NULL;
modbus_mapping_t* RaspiServer::mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
int RaspiServer::server_socket  = -1;
const int RaspiServer::serverid = 6;
uint8_t RaspiServer::query[MODBUS_TCP_MAX_ADU_LENGTH];


// #define NB_CONNECTION       5

// static int server_socket = -1;		//for TCP

RaspiServer::RaspiServer(const int connection_type) :
nb_float  	(2),
nb_unsigned (2),
axis_one_pos		((uint16_t *)malloc(nb_float*sizeof(uint16_t))),
axis_two_pos		((uint16_t *)malloc(nb_float*sizeof(uint16_t))),
axis_three_pos		((uint16_t *)malloc(nb_float*sizeof(uint16_t))),
axis_four_pos		((uint16_t *)malloc(nb_float*sizeof(uint16_t))),
axis_one_dst_pos	((uint16_t *)malloc(nb_float*sizeof(uint16_t))),
axis_two_dst_pos	((uint16_t *)malloc(nb_float*sizeof(uint16_t))),
axis_three_dst_pos	((uint16_t *)malloc(nb_float*sizeof(uint16_t))),
axis_four_dst_pos	((uint16_t *)malloc(nb_float*sizeof(uint16_t))),
nb_connection(5)
{
    if(connection_type == TYPE_RTU)
	{
        printf("sykdebug:rtu type\n");
		if(initRTU() == 0)
        {
			ret = pthread_create(&th, NULL, threadRTU, mb_mapping);
        }
	}
	else if(connection_type == TYPE_TCP)
	{
        printf("sykdebug:tcp type\n");
		if(initTCP() == 0)
			ret = pthread_create(&th, NULL, threadTCP, mb_mapping);

	}
	if( ret != 0 )
	{
	    printf( "Create thread error!\n");
	    return;
	}
    sleep(1);
}

RaspiServer::~RaspiServer()
{
	closeModbus(1);
}


int RaspiServer::initRTU()
{
	
    // printf("sykdebug: begin to create new rtu ctx\n");
	ctx = modbus_new_rtu("/dev/ttyUSB0", 38400, 'E', 8, 1);
    modbus_set_debug(ctx, FALSE);
	mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
	if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

	modbus_rtu_set_serial_mode(ctx, MODBUS_RTU_RS232);
    modbus_set_slave(ctx, RaspiServer::serverid);
	
	if(modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Coneection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }


    // usleep(100);
    return 0;
}

int RaspiServer::initTCP()
{

	ctx = modbus_new_tcp("192.168.142.129", 502);
    /*sykfix: modbus_mapping_new should give rasp pi write bits and write register*/
    mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);

    modbus_set_slave(ctx, RaspiServer::serverid);

    if (mb_mapping == NULL) {
        fprintf(stderr, "Failed to allocate the mapping: %s\n",
                modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    server_socket = modbus_tcp_listen(ctx, nb_connection);
    if (server_socket == -1)
    {
        fprintf(stderr, "Unable to listen TCP connection\n");
        modbus_free(ctx);
        return -1;
    }
    else
		return 0;
}

void * RaspiServer::threadRTU(void *arg)
{
	int rc;
    for(;;) {
        uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];
        rc = modbus_receive(ctx, query);
        if (rc > 0) {
            modbus_reply(ctx, query, rc, mb_mapping);
        } else if (rc  == -1) {
            /* Connection closed by the client or error */
            modbus_close(ctx);
            modbus_free(ctx);

            ctx = modbus_new_rtu("/dev/ttyUSB0", 38400, 'E', 8, 1);
            modbus_set_slave(ctx, RaspiServer::serverid);
            modbus_connect(ctx);

            //break;
        }
    }
    printf("Quit the loop: %s\n", modbus_strerror(errno));

    /* For RTU, skipped by TCP (no TCP connect) */
    modbus_close( ctx);
    modbus_free( ctx);

    // return 0;

}

void * RaspiServer::threadTCP(void *arg)
{
	fd_set 	refset;
    fd_set 	rdset;
    int fdmax;
	int master_socket;
    int rc;
	signal(SIGINT, closeModbus);		//InterruptKey to close the signal

    /* Clear the reference set of socket */
    FD_ZERO(&refset);
    /* Add the server socket */
    FD_SET(server_socket, &refset);

    /* Keep track of the max file descriptor */
    fdmax = server_socket;

    for(;;)
    {
    	rdset = refset;
    	// printf("sykdebug: begin to wait socket\n");
    	/*sykfix: need to add write fdset. And noticed the select function is run by blocked*/
    	if(select(fdmax+1, &rdset, NULL, NULL, NULL) == -1)
    	{
    		perror("Server select() failure.");
    		closeModbus(1);
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
    		}
    	}
    }
}

int RaspiServer::getAxisCurPos(const int &axisnum, float &f_axispos) const
{
	uint16_t * p_axispos = (uint16_t *)malloc(nb_float*sizeof(uint16_t));
	uint16_t addr = -1;
	switch(axisnum)
    {
        case AXIS_ONE:
            addr = ADDR_AXIS_ONE_POS;
            break;
        case AXIS_TWO:
            addr = ADDR_AXIS_TWO_POS;
            break;
        case AXIS_THREE:
            addr = ADDR_AXIS_THREE_POS;
            break;
        case AXIS_FOUR:
            addr = ADDR_AXIS_FOUR_POS;
            break;
        default:
            printf("Error getAxisCurPos: axisnum input is wrong\n");
            return -1;
    }
    p_axispos = &(mb_mapping->tab_registers[addr]);
    f_axispos = modbus_get_float(p_axispos);
    printf("sykdebug: get pos, addr = %d, f_axispos = %f\n", addr, f_axispos);
    return 0;
}

int RaspiServer::setAxisDstPos (const int &axisnum, const float &f_axisdstpos)
{
	uint16_t * p_axisdstpos = (uint16_t *)malloc(nb_float*sizeof(uint16_t));
	uint16_t addr = -1;
	switch(axisnum)
    {
        case AXIS_ONE:
            addr = ADDR_AXIS_ONE_DST_POS;
            break;
        case AXIS_TWO:
            addr = ADDR_AXIS_TWO_DST_POS;
            break;
        case AXIS_THREE:
            addr = ADDR_AXIS_THREE_DST_POS;
            break;
        case AXIS_FOUR:
            addr = ADDR_AXIS_FOUR_DST_POS;
            break;
        default:
            printf("Error getAxisDstPos: axisnum input is wrong\n");
            return -1;
    }
    p_axisdstpos = &(mb_mapping->tab_registers[addr]);
    modbus_set_float(f_axisdstpos, p_axisdstpos);
    printf("sykdebug: addr = %d, f_axisdstpos = %f\n", addr, f_axisdstpos);
    return 0;
}

int RaspiServer::getAxisDstPos(const int &axisnum, float f_axisdstpos) const
{
	uint16_t * p_axisdstpos = (uint16_t *)malloc(nb_float*sizeof(uint16_t));
	uint16_t addr = -1;
	switch(axisnum)
    {
        case AXIS_ONE:
            addr = ADDR_AXIS_ONE_DST_POS;
            break;
        case AXIS_TWO:
            addr = ADDR_AXIS_TWO_DST_POS;
            break;
        case AXIS_THREE:
            addr = ADDR_AXIS_THREE_DST_POS;
            break;
        case AXIS_FOUR:
            addr = ADDR_AXIS_FOUR_DST_POS;
            break;
        default:
            printf("Error getAxisDstPos: axisnum input is wrong\n");
            return -1;
    }
    p_axisdstpos = &(mb_mapping->tab_registers[addr]);
    f_axisdstpos = modbus_get_float(p_axisdstpos);
    printf("sykdebug: addr = %d, f_axisdstpos = %f\n", addr, f_axisdstpos);

    return 0;
}

int RaspiServer::enablePositioning()
{
	uint16_t addr_enablepos = ADDR_ENABLE_POSITIONING;
	uint16_t *p_enablepos = &(mb_mapping->tab_registers[addr_enablepos]);


	/* sykfix: how to define the delaytime's length */
	__useconds_t delaytime = 50;
    const int down = 0x00;
    const int up   = 0xFF;

    *p_enablepos = down;
    usleep(delaytime);

    *p_enablepos = up;
    usleep(delaytime);

    *p_enablepos = down;
    usleep(100);

    /* sykdebug: show the dst pos of axis */
    int axisnum = 0;
    float axisdstpos = 0;
    printf("sykdebug: the dest pos of axis are: \n");
    for(; axisnum < 4; axisnum++)
    {
        getAxisDstPos(axisnum, axisdstpos);
    }

    /* sykfix: need to judge whether finish positionning */

    // while(getPositioningFlag() != 1)
    // {
    	
    // }
    return 0;
}

uint16_t RaspiServer::getPositioningFlag() const
{
    uint16_t addr_posflag = ADDR_POSITIONING_FLAG;
    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_posflag]);
    return *p_posflag;
}

int RaspiServer::setCameraPos(const int & rotateflag)
{
    uint16_t addr_camposflag = ADDR_CAMERA_ROTATE;
    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_camposflag]);
    switch(rotateflag)
    {
        case CAM_DOWN:
            *p_posflag = 1;
            break;
        case CAM_UP:
            *p_posflag = 0;
            break;
        default:
            printf("Error setCameraPos: rotateflag is wrong\n");
            return -1;
    }
    return 0;
}

int RaspiServer::setClawPos(const int &clawindex, const int &clawflag)
{
    uint16_t addr_clawposflag;
    switch(clawindex)
    {
        case MATERIAL_CLAW:
            addr_clawposflag = ADDR_MATERIAL_CLAW_CONTROL;
            break;
        case PRODUCT_CLAW:
            addr_clawposflag = ADDR_PRODUCT_CLAW_CONTROL;
            break;
        default:
            printf("Error: clawindex is wrong\n");
            return -1;
    }

    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_clawposflag]);
    /* sykfix: need to add judgement of claw's status */
    switch(addr_clawposflag)
    {
        case CLAW_HOLDING:
            *p_posflag = 0;
            break;
        case CLAW_GRASP:
            *p_posflag = 1;
            break;
        case CLAW_LOOSE:
            *p_posflag = 2;
            break;
        default:
            printf("Error setClawPos: clawflag is wrong\n");
            return -1;
    }
    return 0;
}

int RaspiServer::getClawPos(const int &clawindex) const
{
    uint16_t addr_clawposflag;
    switch(clawindex)
    {
        case MATERIAL_CLAW:
            addr_clawposflag = ADDR_MATERIAL_CLAW_STATUS;
            break;
        case PRODUCT_CLAW:
            addr_clawposflag = ADDR_PRODUCT_CLAW_STATUS;
            break;
        default:
            printf("Error: clawindex is wrong\n");
            return -1;
    }
    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_clawposflag]);
    return *p_posflag;
}

void RaspiServer::closeModbus(int dummy)
{
	if (server_socket != -1) {
        close(server_socket);
    }
    modbus_close(ctx);
    modbus_free(ctx);
    modbus_mapping_free(mb_mapping);

    exit(dummy);
}
