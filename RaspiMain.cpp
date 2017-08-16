#include "Raspi.h"

int main()
{
	RaspiServer server = RaspiServer(TYPE_RTU);

	int axisnum   = AXIS_X;
	float axispos = 1.0;

	/* Test write axis and enable them */
	// for(axisnum = AXIS_ONE, axispos = 1.0; axisnum <= AXIS_FOUR; axisnum++, axispos++)
	// 	server.setAxisDstPos(axisnum, axispos);
	// server.enablePositioning();

	/* Test read axis */
	for(axisnum = AXIS_X; axisnum <= AXIS_U; axisnum++)
		server.getAxisCurPos(axisnum, axispos);

	/* Test read claw pos */
	std::cout << server.getClawStatus(MATERIAL_CLAW) << std::endl;
	std::cout << server.getClawStatus(PRODUCT_CLAW) << std::endl;

	/* Test set camera pos */
	std::cout << server.setCameraStatus(CAM_DOWN) << std::endl;

	/* Test get ownership */
	server.setOwnership(1);

	while(1)
	{
		// TODO
	}

	return 0;
}