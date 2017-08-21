#include "Raspi.h"

modbus_t* RaspiServer::ctx = NULL;
modbus_mapping_t* RaspiServer::mb_mapping = modbus_mapping_new(MODBUS_MAX_READ_BITS, 0,
                                    MODBUS_MAX_READ_REGISTERS, 0);
int RaspiServer::server_socket  = -1;
const int RaspiServer::serverid = 6;
uint8_t RaspiServer::query[MODBUS_TCP_MAX_ADU_LENGTH];


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
nb_connection   (5),
axis_x_org      (-180),
axis_y_org      (-260),
axis_z_org      (595),
axis_u_org      (0)
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
    sleep(8);
    printf("Construction function over\n");
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
    // modbus_set_debug(ctx, TRUE);
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
            // modbus_set_debug(ctx, TRUE);
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

int RaspiServer::Init()
{
    setCameraStatus(CAM_DOWN);
    setClawStatus(MATERIAL_CLAW, CLAW_GRASP);
    setClawStatus(PRODUCT_CLAW, CLAW_LOOSE);

    /* sykfix: add cam position */
    setAxisDstPos(AXIS_X, 0);
    setAxisDstPos(AXIS_Y, 0);
    setAxisDstPos(AXIS_Z, 0);
    setAxisDstPos(AXIS_U, 0);
    enablePositioning();

    return 0;
}

int RaspiServer::getAxisCurPos(const int &axisnum, float &f_axispos) const
{
	uint16_t * p_axispos = (uint16_t *)malloc(nb_float*sizeof(uint16_t));
    uint16_t addr = -1;
    float org     = 0;
    int sign      = 1;       //for axis_z, sign need to be -1
    char axisname;
	switch(axisnum)
    {
        case AXIS_X:
            addr = ADDR_AXIS_X_POS;
            org = axis_x_org;
            axisname = 'X';
            break;
        case AXIS_Z:
            addr = ADDR_AXIS_Z_POS;
            org = axis_z_org;
            sign = -1;
            axisname = 'Z';
            break;
        case AXIS_U:
            addr = ADDR_AXIS_U_POS;
            org = axis_u_org;
            axisname = 'U';
            break;
        case AXIS_Y:
            addr = ADDR_AXIS_Y_POS;
            org = axis_y_org;
            axisname = 'Y';
            break;
        default:
            printf("Error getAxisCurPos: axisnum input is wrong\n");
            return -1;
    }
    p_axispos = &(mb_mapping->tab_registers[addr]);
    f_axispos = modbus_get_float(p_axispos);
    // printf("sykdebug: get axis raw pos, addr = %d, rawpos = %f\n", addr, f_axispos);
    f_axispos = sign*(f_axispos - org);
    // printf("sykdebug: get axis world pos, for axis %c, worldpos = %f\n", axisname, f_axispos);
    return 0;
}

int RaspiServer::setAxisDstPos (const int &axisnum, const float &f_axisdstpos)
{
	uint16_t * p_axisdstpos = (uint16_t *)malloc(nb_float*sizeof(uint16_t));
	uint16_t addr = -1;
    float org     = 0;
    int sign      = 1;       //for axis_z, sign need to be -1
    char axisname;
	switch(axisnum)
    {
        case AXIS_X:
            addr = ADDR_AXIS_X_DST_POS;
            org = axis_x_org;
            axisname = 'X';
            break;
        case AXIS_Z:
            addr = ADDR_AXIS_Z_DST_POS;
            org = axis_z_org;
            sign = -1;
            axisname = 'Z';
            break;
        case AXIS_U:
            addr = ADDR_AXIS_U_DST_POS;
            org = axis_u_org;
            axisname = 'U';
            break;
        case AXIS_Y:
            addr = ADDR_AXIS_Y_DST_POS;
            org = axis_y_org;
            axisname = 'Y';
            break;
        default:
            printf("Error getAxisDstPos: axisnum input is wrong\n");
            return -1;
    }
    p_axisdstpos = &(mb_mapping->tab_registers[addr]);
    float tmpdstpos;
    tmpdstpos = sign*f_axisdstpos + org;
    modbus_set_float(tmpdstpos, p_axisdstpos);
    printf("sykdebug: set axis raw pos, addr = %d, rawpos = %f\n", addr, tmpdstpos);
    // printf("sykdebug: set axis world pos, for axis %c, worldpos = %f\n", axisname, f_axisdstpos);
    return 0;
}

int RaspiServer::getAxisDstPos(const int &axisnum, float &f_axisdstpos) const
{
	uint16_t * p_axisdstpos = (uint16_t *)malloc(nb_float*sizeof(uint16_t));
	uint16_t addr = -1;
    float org     = 0;
    int sign      = 1;       //for axis_z, sign need to be -1
    char axisname;
	switch(axisnum)
    {
        case AXIS_X:
            addr = ADDR_AXIS_X_DST_POS;
            org = axis_x_org;
            axisname = 'X';
            break;
        case AXIS_Z:
            addr = ADDR_AXIS_Z_DST_POS;
            org = axis_z_org;
            sign = -1;
            axisname = 'Z';
            break;
        case AXIS_U:
            addr = ADDR_AXIS_U_DST_POS;
            org = axis_u_org;
            axisname = 'U';
            break;
        case AXIS_Y:
            addr = ADDR_AXIS_Y_DST_POS;
            org = axis_y_org;
            axisname = 'Y';
            break;
        default:
            printf("Error getAxisDstPos: axisnum input is wrong\n");
            return -1;
    }
    p_axisdstpos = &(mb_mapping->tab_registers[addr]);
    f_axisdstpos = modbus_get_float(p_axisdstpos);
    printf("sykdebug: get axis dst raw pos, addr = %d, rawpos = %f\n", addr, f_axisdstpos);
    f_axisdstpos = sign*(f_axisdstpos - org);
    printf("sykdebug: get axis dst world pos, for axis %c, worldpos = %f\n", axisname, f_axisdstpos);
    return 0;
}

int RaspiServer::enablePositioning()
{
	uint16_t addr_enablepos = ADDR_ENABLE_POSITIONING;
	uint16_t *p_enablepos = &(mb_mapping->tab_registers[addr_enablepos]);


	/* sykfix: how to define the delaytime's length */
	unsigned int delaytime = 100 * 1000;

    const uint16_t down = 0;
    const uint16_t up   = 1;

    // sleep(1);

    *p_enablepos = down;
    usleep(delaytime);
    printf("sykdebug: *p_enablepos = %d\n", *p_enablepos);

    *p_enablepos = up;
    usleep(delaytime);
    printf("sykdebug: *p_enablepos = %d\n", *p_enablepos);

    *p_enablepos = down;

    /* sykdebug: show the dst pos of axis */
    int axisnum = 0;
    float axisdstpos = 0;
    printf("sykdebug: the dest pos of axis are: \n");
    for(; axisnum < 4; axisnum++)
    {
        getAxisDstPos(axisnum, axisdstpos);
    }

    /* sykfix: need to judge whether finish positionning */

    // while(getUpPosFlag() != 1)
    // {
    	
    // }
    return 0;
}

int RaspiServer::getDownPosFlag() const
{
    uint16_t addr_posflag = ADDR_POSITIONING_FLAG;
    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_posflag]);
    return *p_posflag;
}

int RaspiServer::getDownIdentifyFlag() const
{
    uint16_t addr_posflag = ADDR_IDENTIFYING_FLAG;
    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_posflag]);
    return *p_posflag;
}

int RaspiServer::getUpPosFlag() const
{
    uint16_t addr_posflag = ADDR_FINISH_IDENTIFYING;
    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_posflag]);
    return *p_posflag;
}

int RaspiServer::setUpPosFlag(const int &posflag)
{
    uint16_t addr_posflag = ADDR_FINISH_IDENTIFYING;
    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_posflag]);
    *p_posflag = posflag;
    return 0;
}

int RaspiServer::setCameraStatus(const int & rotateflag)
{
    uint16_t addr_camposflag = ADDR_CAMERA_ROTATE;
    uint16_t *p_posflag = &(mb_mapping->tab_registers[addr_camposflag]);
    switch(rotateflag)
    {
        case CAM_DOWN:
            *p_posflag = 0;
            break;
        case CAM_UP:
            *p_posflag = 1;
            break;
        default:
            printf("Error setCameraPos: rotateflag is wrong\n");
            return -1;
    }
    return 0;
}

int RaspiServer::setClawStatus(const int &clawindex, const int &clawflag)
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
    switch(clawflag)
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

int RaspiServer::getClawStatus(const int &clawindex) const
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

int RaspiServer::setCameraPos(const float &x, const float &y, const float &z)
{
    setAxisDstPos(AXIS_X, x);
    setAxisDstPos(AXIS_Y, y);
    setAxisDstPos(AXIS_Z, z);
    enablePositioning();
    return 0;
}

int RaspiServer::getCameraPos(float &x, float &y, float &z) const
{
    getAxisCurPos(AXIS_X, x);
    getAxisCurPos(AXIS_Y, y);
    getAxisCurPos(AXIS_Z, z);
    int positionningflag;
    // positionningflag = getUpPosFlag();
    float tmpx, tmpy, tmpz;
    getCameraDstPos(tmpx, tmpy, tmpz);
    if(tmpx == x && tmpy == tmpy && tmpz == tmpz)
    {
        positionningflag = 1;

    }
    else
        positionningflag = 0;

    if(positionningflag == 0)
        printf("Still Positionning: ");
    else
        printf("Finish Positionning: ");
    printf("X = %.2f, Y = %.2f, Z = %.2f\n", x, y, z);
    return 0;
}

int RaspiServer::getCameraDstPos(float &x, float &y, float &z) const
{
    getAxisDstPos(AXIS_X, x);
    getAxisDstPos(AXIS_Y, y);
    getAxisDstPos(AXIS_Z, z);
    return 0;
}