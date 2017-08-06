#ifndef _RASPI_H_
#define _RASPI_H_

/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/*
    Used on Raspeberry Pi, as a master to write or read the register/coils of HMI. 
    Use TCP/IP to connect with HMI.
*/

#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <modbus/modbus.h>

/* Register of HMI(for Raspi to read), saving current status of different parameters */
#define ADDR_AXIS_ONE_POS            0       //FLAOT
#define ADDR_AXIS_TWO_POS            2       //FLAOT
#define ADDR_AXIS_THREE_POS          4       //FLAOT
#define ADDR_AXIS_FOUR_POS           6       //FLAOT
#define ADDR_CLAW_MATERIALS_STATUS   8       //Us 1-Clamp 0-Loose      
#define ADDR_CLAW_PRODUCTS_STATUS    9       //Us 1-Clamp 0-Loose      
#define ADDR_POSITIONING_FLAG        10      //Us 1-Positioning 0-Normal status
#define ADDR_IDENTIFYING_FLAG        11      //Us 1-Enable identifying module 0-Unable

/* Register of HMI(for Raspi to write) */
#define ADDR_AXIS_ONE_DST_POS        100     //FLAOT
#define ADDR_AXIS_TWO_DST_POS        102     //FLAOT
#define ADDR_AXIS_THREE_DST_POS      104     //FLAOT
#define ADDR_AXIS_FOUR_DST_POS       106     //FLAOT
#define ADDR_BEGIN_POSITIONING       108     //US 0->1 begin positioning
#define ADDR_CAMERA_ROTATE           109     //US 0-Up 1-Down
#define ADDR_CLAW_MATERIALS_CONTROL  110     //US 0-Keep 1-Loose
#define ADDR_CLAW_PRODUCT_CONTROL    111     //US 0-Keep 1-Loose
#define ADDR_FINISH_IDENTIFYING      112     //US 0-Identifying 1-Finished 2-Failed

enum AXIS_NUM
{
    AXIS_ONE,
    AXIS_TWO,
    AXIS_THREE,
    AXIS_FOUR
};

class RaspiModbus
{
public:	
	RaspiModbus();
	~RaspiModbus();
	int initModbusTCP();
	int closeModbusTCP();

	int readAxisPos(const int &axisnum, float &f_axispos);
	int writeAxisPos(const int &axisnum, float &f_axispos);
	int enablePositioning();

private:
	static modbus_t* ctx;
	static uint16_t *axis_one_pos  ;
	static uint16_t *axis_two_pos  ;
	static uint16_t *axis_three_pos;
	static uint16_t *axis_four_pos ;

	static uint16_t *axis_one_dst_pos  ;
	static uint16_t *axis_two_dst_pos  ;
	static uint16_t *axis_three_dst_pos;
	static uint16_t *axis_four_dst_pos ;

	static const int slaveid;

};



#endif