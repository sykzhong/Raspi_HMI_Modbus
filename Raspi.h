/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

/*
    Used on Raspi, as a slave.
    Use RTU to connect with robot controller.
*/

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>

#include <modbus/modbus.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* Register of Raspi, saving current status of different parameters */
#define ADDR_AXIS_ONE_POS            0       //FLOAT
#define ADDR_AXIS_TWO_POS            2       //FLOAT
#define ADDR_AXIS_THREE_POS          4       //FLOAT
#define ADDR_AXIS_FOUR_POS           6       //FLOAT
#define ADDR_CLAW_MATERIALS_STATUS   8       //US 1-Clamp 0-Loose      
#define ADDR_CLAW_PRODUCTS_STATUS    9       //US 1-Clamp 0-Loose      
#define ADDR_POSITIONING_FLAG        10      //US 1-Positioning 0-Normal status
#define ADDR_IDENTIFYING_FLAG        11      //US 1-Enable identifying module 0-Unable

/* Register of Raspi, saving destination or indiction for controller */
#define ADDR_AXIS_ONE_DST_POS        100     //FLOAT
#define ADDR_AXIS_TWO_DST_POS        102     //FLOAT
#define ADDR_AXIS_THREE_DST_POS      104     //FLOAT
#define ADDR_AXIS_FOUR_DST_POS       106     //FLOAT
#define ADDR_ENABLE_POSITIONING      108     //US 0->1 enable positioning
#define ADDR_CAMERA_ROTATE           109     //US 0-Up 1-Down
#define ADDR_CLAW_MATERIALS_CONTROL  110     //US 0-Keep 1-Loose
#define ADDR_CLAW_PRODUCT_CONTROL    111     //US 0-Keep 1-Loose
#define ADDR_FINISH_IDENTIFYING      112     //US 0-Identifying 1-Finished 2-Failed

enum AXIS_NUM
{
    AXIS_ONE = 0,
    AXIS_TWO,
    AXIS_THREE,
    AXIS_FOUR
};

enum CONNECTION_TYPE
{
	TYPE_RTU,
	TYPE_TCP
};

class RaspiServer
{
public:
	RaspiServer(const int connection_type = TYPE_RTU);
	~RaspiServer();
	int initRTU();
	int initTCP();
	int getAxisCurPos (const int &axisnum, float &f_axispos)   	const;
	int getAxisDstPos (const int &axisnum, float f_axisdstpos) 	const;
	int putAxisDstPos (const int &axisnum, const float &f_axisdstpos);
	static void closeModbus (const int dummy);

	static void * threadRTU(void *arg);
	static void * threadTCP(void *arg);

	int enablePositioning();
private:
	static modbus_t* ctx;
	static modbus_mapping_t *mb_mapping;
	
	uint16_t *axis_one_pos  ;
	uint16_t *axis_two_pos  ;
	uint16_t *axis_three_pos;
	uint16_t *axis_four_pos ;

	uint16_t *axis_one_dst_pos  ;
	uint16_t *axis_two_dst_pos  ;
	uint16_t *axis_three_dst_pos;
	uint16_t *axis_four_dst_pos ;

	const int serverid;

	int nb_float;			//register number of float data
	int nb_unsigned;		//register number of unsigned data

	const int nb_connection;	//use in TCP connection
	static int server_socket;	//use in TCP connection

	static uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];	//use to save indiction

	pthread_t th;
	int ret;
};