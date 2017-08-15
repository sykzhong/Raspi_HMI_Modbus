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

#include <iostream>

/* sykfix: need to change axis addr to x y z theta */
/* Register of Raspi, saving current status of different parameters */
#define ADDR_AXIS_ONE_POS          		0       // 4x FLOAT
#define ADDR_AXIS_TWO_POS          		2       // 4x FLOAT
#define ADDR_AXIS_THREE_POS        		4       // 4x FLOAT
#define ADDR_AXIS_FOUR_POS         		6       // 4x FLOAT
#define ADDR_MATERIAL_CLAW_STATUS  		8       // 4x US 1-Clamp 0-Loose      
#define ADDR_PRODUCT_CLAW_STATUS   		9       // 4x US 1-Clamp 0-Loose      
#define ADDR_POSITIONING_FLAG      		10      // 4x US 1-Positioning 0-Normal status
#define ADDR_IDENTIFYING_FLAG      		11      // 4x US 1-Enable identifying module 0-Unable

/* Register of Raspi, saving destination or indiction for controller */
#define ADDR_AXIS_ONE_DST_POS      		100     // 4x FLOAT
#define ADDR_AXIS_TWO_DST_POS      		102     // 4x FLOAT
#define ADDR_AXIS_THREE_DST_POS    		104     // 4x FLOAT
#define ADDR_AXIS_FOUR_DST_POS     		106     // 4x FLOAT
#define ADDR_ENABLE_POSITIONING    		108     // 4x US 0->1 enable positioning
#define ADDR_CAMERA_ROTATE         		109     // 4x US 0-Up 1-Down
#define ADDR_MATERIAL_CLAW_CONTROL 		110     // 4x US 0-Keep 1-Loose
#define ADDR_PRODUCT_CLAW_CONTROL  		111     // 4x US 0-Keep 1-Loose
#define ADDR_FINISH_IDENTIFYING    		112     // 4x US 0-Identifying 1-Finished 2-Failed

#define ADDR_OWNERSHIP					11		// 0x US 0-no ownership 1-get ownership	

enum AXIS_NUM
{
    AXIS_ONE = 0,
    AXIS_TWO,
    AXIS_THREE,
    AXIS_FOUR
};

enum CLAW_INDEX
{
	MATERIAL_CLAW,
	PRODUCT_CLAW
};

enum CLAW_STATUS
{
	CLAW_HOLDING,
	CLAW_GRASP,
	CLAW_LOOSE
};

enum CAM_STATUS
{
	CAM_DOWN,
	CAM_UP,
};

enum OWNERSHIP_FLAG
{
	SET_OWNERSHIP,
	UNSET_OWNERSHIP
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
	static void * threadRTU(void *arg);
	static void * threadTCP(void *arg);

	int getAxisCurPos 	(const int &axisnum, float &f_axispos)   	const;
	int getAxisDstPos 	(const int &axisnum, float f_axisdstpos) 	const;
	int setAxisDstPos 	(const int &axisnum, const float &f_axisdstpos);
	int enablePositioning();
	int getPositioningFlag() const;

	int setCameraStatus	(const int &rotateflag);
	int getClawStatus	(const int &clawindex) const;
	int setClawStatus	(const int &clawindex, const int &clawflag);


	int setOwnership	(const int ownershipflag);		//sykfix

	int Init();

	static void closeModbus (const int dummy);


private:
	static modbus_t* ctx;
	static modbus_mapping_t *mb_mapping;
	static uint8_t query[MODBUS_TCP_MAX_ADU_LENGTH];	//use to save indiction
	static const int serverid;

	pthread_t th;
	int ret;

	uint16_t *axis_one_pos  ;
	uint16_t *axis_two_pos  ;
	uint16_t *axis_three_pos;
	uint16_t *axis_four_pos ;

	uint16_t *axis_one_dst_pos  ;
	uint16_t *axis_two_dst_pos  ;
	uint16_t *axis_three_dst_pos;
	uint16_t *axis_four_dst_pos ;

	const float axis_x_org;
	const float axis_y_org;
	const float axis_z_org;
	const float axis_theta_org;

	int nb_float;				//register number of float data
	int nb_unsigned;			//register number of unsigned data

	const int nb_connection;	//used in TCP connection
	static int server_socket;	//used in TCP connection

};