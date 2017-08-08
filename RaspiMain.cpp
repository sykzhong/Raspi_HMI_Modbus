#include "Raspi.h"

int main()
{
	RaspiServer server = RaspiServer(TYPE_TCP);
	int axisnum = AXIS_ONE;
	float axispos = -1.0;
	server.getAxisCurPos(AXIS_ONE, axispos);
}