#include "Raspi.h"

int main()
{
    RaspiModbus raspi;


    /* Test write modbus */
    // float dstpos = 18.0;
    // int axisnum = AXIS_ONE;
    // int rc;

    // raspi.writeAxisPos(axisnum, dstpos);
    // raspi.enablePositioning();

    /* Test read modbus */
    float dstpos = 0.0;
    int axisnum = AXIS_ONE;
    int rc;

    rc = raspi.readAxisPos(axisnum, dstpos);
    printf("dstpos = %f\n", dstpos);

}