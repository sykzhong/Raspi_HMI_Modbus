#include "Raspi.h"

int main()
{
	RaspiServer server = RaspiServer(TYPE_RTU);
	// server.Init();

	int axisnum   = AXIS_X;
	float axispos = 1.0;

	/* Test write axis and enable them */
	for(axisnum = AXIS_X, axispos = 1.0; axisnum <= AXIS_U; axisnum++, axispos++)
		server.setAxisDstPos(axisnum, 40);
	server.enablePositioning();

	/* Test read axis */
	// for(axisnum = AXIS_X; axisnum <= AXIS_U; axisnum++)
	// 	server.getAxisCurPos(axisnum, axispos);

	/* Test read claw pos */
	// std::cout << server.getClawStatus(MATERIAL_CLAW) << std::endl;
	// std::cout << server.getClawStatus(PRODUCT_CLAW) << std::endl;

	/* Test set camera pos */
	// std::cout << server.setCameraStatus(CAM_DOWN) << std::endl;

	/* Test getCameraPos */
	// float x = 10, y = 10, z = 10;
	// server.getCameraPos(x, y, z);
	// server.setCameraPos(x, y, z);
	while(1)
	{
		std::cout << "DownPosFlag = " << server.getDownPosFlag() << std::endl;
		std::cout << "DownIdentifyFlag = " << server.getDownIdentifyFlag() << std::endl;
		sleep(1);
		// TODO
	}

	return 0;
}