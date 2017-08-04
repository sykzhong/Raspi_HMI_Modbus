
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
#define ADDR_AXIS_ONE_DST_POS       100     //FLAOT
#define ADDR_AXIS_TWO_DST_POS       102     //FLAOT
#define ADDR_AXIS_THREE_DST_POS     104     //FLAOT
#define ADDR_AXIS_FOUR_DST_POS      106     //FLAOT
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

/* Parameters for initialization */
#define SLAVE_ID            6
static modbus_t* ctx = NULL;

/* Registers of 4 axis, each register contain 4 bytes */
static uint16_t *axis_one_pos   = (uint16_t *)malloc(2*sizeof(uint16_t));
static uint16_t *axis_two_pos   = (uint16_t *)malloc(2*sizeof(uint16_t));
static uint16_t *axis_three_pos = (uint16_t *)malloc(2*sizeof(uint16_t));
static uint16_t *axis_four_pos  = (uint16_t *)malloc(2*sizeof(uint16_t));

static uint16_t *axis_one_dst_pos   = (uint16_t *)malloc(2*sizeof(uint16_t));
static uint16_t *axis_two_dst_pos   = (uint16_t *)malloc(2*sizeof(uint16_t));
static uint16_t *axis_three_dst_pos = (uint16_t *)malloc(2*sizeof(uint16_t));
static uint16_t *axis_four_dst_pos  = (uint16_t *)malloc(2*sizeof(uint16_t));

int initModbusTCP()
{
    // ctx = modbus_new_tcp("192.168.123.156", 502);           //Connect to windows
    ctx = modbus_new_tcp("192.168.142.129", 502);           //Connect to Ubuntu
    modbus_set_debug(ctx, TRUE);
    modbus_set_slave(ctx, SLAVE_ID);       //If slave use slave ID, then there need to set too

    if(modbus_connect(ctx) == -1)
    {
        fprintf(stderr, "Coneection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return -1;
    }

    int nb = 2;

    memset(axis_one_pos, 0, nb*sizeof(uint16_t));
    memset(axis_two_pos, 0, nb*sizeof(uint16_t));
    memset(axis_three_pos, 0, nb*sizeof(uint16_t));
    memset(axis_four_pos, 0, nb*sizeof(uint16_t));

    memset(axis_one_dst_pos, 0, nb*sizeof(uint16_t));
    memset(axis_two_dst_pos, 0, nb*sizeof(uint16_t));
    memset(axis_three_dst_pos, 0, nb*sizeof(uint16_t));
    memset(axis_four_dst_pos, 0, nb*sizeof(uint16_t));

    return 0;
}

int readAxisPos(int &axisnum, float &f_axispos)
{
    int rc;
    uint16_t addr = 0;
    uint16_t *p_axispos = (uint16_t *)malloc(2*sizeof(uint16_t));
    int nb = 2;
    switch(axisnum)
    {
        case AXIS_ONE:  
            addr = ADDR_AXIS_ONE_POS;   
            p_axispos = axis_one_pos;    
            break;
        case AXIS_TWO:  
            addr = ADDR_AXIS_TWO_POS;
            p_axispos = axis_two_pos;   
            break;
        case AXIS_THREE:
            addr = ADDR_AXIS_THREE_POS;
            p_axispos = axis_three_pos;   
            break;
        case AXIS_FOUR: 
            addr = ADDR_AXIS_FOUR_POS;
            p_axispos = axis_four_pos;   
            break;
    }

    rc = modbus_read_registers(ctx, addr, nb, p_axispos);
    if (rc != nb) 
    {
        printf("ERROR modbus_read_registers single (%d)\n", rc);
        printf("Address = %d\n", addr);
        return -1;
    }
    f_axispos = modbus_get_float_abcd(p_axispos);
    return 0;
}

int writeAxisPos(int &axisnum, float &f_axispos)
{
    int rc;
    uint16_t addr = 0;
    uint16_t *p_axisdstpos = (uint16_t *)malloc(2*sizeof(uint16_t));
    int nb = 2;
    switch(axisnum)
    {
        case AXIS_ONE:  
            addr = ADDR_AXIS_ONE_DST_POS;   
            p_axisdstpos = axis_one_dst_pos;    
            break;
        case AXIS_TWO:  
            addr = ADDR_AXIS_TWO_DST_POS;
            p_axisdstpos = axis_two_dst_pos;   
            break;
        case AXIS_THREE:
            addr = ADDR_AXIS_THREE_DST_POS;
            p_axisdstpos = axis_three_dst_pos;   
            break;
        case AXIS_FOUR: 
            addr = ADDR_AXIS_FOUR_DST_POS;
            p_axisdstpos = axis_four_dst_pos;   
            break;
    }

    modbus_set_float_abcd(f_axispos, p_axisdstpos);

    rc = modbus_write_registers(ctx, addr, nb, p_axisdstpos);
    if (rc != nb) {
        printf("ERROR modbus_write_registers (%d)\n", rc);
        printf("Address = %d, nb = %d\n", addr, nb);
        return -1;
    }
    return 0;
}

int main()
{
    initModbusTCP();

    /* Test write modbus */
    float dstpos = 18.0;
    int axisnum = AXIS_ONE;
    int rc;

    writeAxisPos(axisnum, dstpos);


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