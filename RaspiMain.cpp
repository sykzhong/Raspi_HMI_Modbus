#include "Raspi.h"

int main()
{
	RaspiServer server = RaspiServer(TYPE_RTU);

	int axisnum   = AXIS_ONE;
	float axispos = 1.0;

	/* Test write axis and enable them */
	for(axisnum = AXIS_ONE, axispos = 1.0; axisnum <= AXIS_FOUR; axisnum++, axispos++)
		server.setAxisDstPos(axisnum, axispos);
	server.enablePositioning();

	/* Test read axis */
	// for(axisnum = AXIS_ONE; axisnum <= AXIS_FOUR; axisnum++)
	// 	server.getAxisCurPos(axisnum, axispos);

	/* Test read claw pos */
	std::cout << server.getClawPos(MATERIAL_CLAW) << std::endl;
	std::cout << server.getClawPos(PRODUCT_CLAW) << std::endl;

	/* Test set camera pos */
	std::cout << server.setCameraPos(CAM_UP) << std::endl;

	/* Test get ownership */
	server.setOwnership(1);

	while(1)
	{
		// TODO
	}

	return 0;
}