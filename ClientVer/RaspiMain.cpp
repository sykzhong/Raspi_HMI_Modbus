#include "Raspi.h"

int main()
{
    RaspiModbus raspi;

    printf("sykdebug: wait for 500us\n");
    usleep(500);
    /* Test write register */
    // float dstpos = 18.0;
    // int axisnum = AXIS_ONE;
    // int rc;

    // raspi.writeAxisPos(axisnum, dstpos);
    // raspi.enablePositioning();

    /* Test read register */
    float dstpos = 0.0;
    int axisnum = AXIS_ONE;
    int rc;
    rc = raspi.readAxisPos(axisnum, dstpos);
    printf("dstpos = %f\n", dstpos);

    /* Test read bit */
    // raspi.readBit();

    /* Test as slave */
    // raspi.slave();



}