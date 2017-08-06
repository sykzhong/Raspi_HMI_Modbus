#include "Raspi.h"

int main()
{
    RaspiModbus raspi;


    /* Test write modbus */
    float dstpos = 18.0;
    int axisnum = AXIS_ONE;
    int rc;

    raspi.writeAxisPos(axisnum, dstpos);
    raspi.enablePositioning();

    /*sykdebug*/
    // float writeresult;
    // readAxisPos(5, writeresult);

}