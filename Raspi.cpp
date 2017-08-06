
/*
 * Copyright © 2008-2014 Stéphane Raimbault <stephane.raimbault@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


/*
    Used on Raspeberry Pi, as a master to write or read the register/coils of HMI. 
    Use TCP/IP to connect with HMI.
*/

#include "Raspi.h"

modbus_t* RaspiModbus::ctx = NULL;

/* Registers of 4 axis, each register contain 4 bytes */
uint16_t * RaspiModbus::axis_one_pos   = (uint16_t *)malloc(2*sizeof(uint16_t));
uint16_t * RaspiModbus::axis_two_pos   = (uint16_t *)malloc(2*sizeof(uint16_t));
uint16_t * RaspiModbus::axis_three_pos = (uint16_t *)malloc(2*sizeof(uint16_t));
uint16_t * RaspiModbus::axis_four_pos  = (uint16_t *)malloc(2*sizeof(uint16_t));

uint16_t * RaspiModbus::axis_one_dst_pos   = (uint16_t *)malloc(2*sizeof(uint16_t));
uint16_t * RaspiModbus::axis_two_dst_pos   = (uint16_t *)malloc(2*sizeof(uint16_t));
uint16_t * RaspiModbus::axis_three_dst_pos = (uint16_t *)malloc(2*sizeof(uint16_t));
uint16_t * RaspiModbus::axis_four_dst_pos  = (uint16_t *)malloc(2*sizeof(uint16_t));

const int RaspiModbus::slaveid = 6;

RaspiModbus::RaspiModbus()
{
    initModbusTCP();
}

RaspiModbus::~RaspiModbus()
{
    closeModbusTCP();
}

int RaspiModbus::closeModbusTCP()
{
    free(axis_one_pos);
    free(axis_two_pos);
    free(axis_three_pos);
    free(axis_four_pos);
    free(axis_one_dst_pos);
    free(axis_two_dst_pos);
    free(axis_three_dst_pos);
    free(axis_four_dst_pos);

    modbus_close(ctx);
    modbus_free(ctx);

    return 1;
}

int RaspiModbus::initModbusTCP()
{
    // ctx = modbus_new_tcp("192.168.123.156", 502);           //Connect to windows
    ctx = modbus_new_tcp("192.168.142.129", 502);           //Connect to Ubuntu
    modbus_set_debug(ctx, TRUE);
    modbus_set_slave(ctx, slaveid);       //If slave use slave ID, then there need to set too

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

int RaspiModbus::readAxisPos(const int &axisnum, float &f_axispos)
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
        default:    
        /*sykdebug: for confirm the write function*/
            // addr = ADDR_AXIS_ONE_DST_POS;
            // p_axispos = axis_one_dst_pos;
            // break;
            printf("axisnum input is wrong\n");
            return -1;
    }

    rc = modbus_read_registers(ctx, addr, nb, p_axispos);
    if (rc != nb) 
    {
        printf("ERROR modbus_read_registers single (%d)\n", rc);
        printf("Address = %d\n", addr);
        return -1;
    }
    f_axispos = modbus_get_float_abcd(p_axispos);
    printf("sykdebug: f_axispos = %f", f_axispos);

    return 0;
}

int RaspiModbus::writeAxisPos(const int &axisnum, float &f_axispos)
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
        default:    
            printf("axisnum input is wrong\n");
            return -1;
    }

    modbus_set_float_abcd(f_axispos, p_axisdstpos);

    rc = modbus_write_registers(ctx, addr, nb, p_axisdstpos);
    if (rc != nb) 
    {
        printf("ERROR modbus_write_registers (%d)\n", rc);
        printf("Address = %d, nb = %d\n", addr, nb);
        return -1;
    }
    return 0;
}

int RaspiModbus::enablePositioning()
{
    int rc;
    uint16_t addr = ADDR_BEGIN_POSITIONING;
    const int down =  0;
    const int up = 1;
    rc = modbus_write_bit(ctx, addr, down);
    if(rc != 1)
    {
        printf("ERROR modbus_write_bit (%d)\n", rc);
        printf("Address = %d, value = %d\n", addr, down);
        return -1;
    }
    usleep(50);
    rc = modbus_write_bit(ctx, addr, up);
    if(rc != 1)
    {
        printf("ERROR modbus_write_bit (%d)\n", rc);
        printf("Address = %d, value = %d\n", addr, up);
        return -1;
    }
    usleep(50);
    rc = modbus_write_bit(ctx, addr, down);
    if(rc != 1)
    {
        printf("ERROR modbus_write_bit (%d)\n", rc);
        printf("Address = %d, value = %d\n", addr, down);
        return -1;
    }

    return 1;
}
