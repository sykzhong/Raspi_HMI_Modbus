#include "Raspi.h"

#define TEST2

#ifdef TEST1
int main()
{
	RaspiServer server = RaspiServer(TYPE_RTU);
	server.Init();

	// server.setUpPosFlag(0);
	// int axisnum   = AXIS_X;
	// float axispos = 1.0;

	// /* Test write axis and enable them */
	// for(axisnum = AXIS_X, axispos = 1.0; axisnum <= AXIS_Z; axisnum++, axispos++)
	// 	server.setAxisDstPos(axisnum, 100);
	// // sleep(2);
	// server.enablePositioning();
	// server.setUpPosFlag(1);
	
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
		std::cout << "UpPosFlag = " << server.getUpPosFlag() << std::endl;
		std::cout << "DownPosFlag = " << server.getDownPosFlag() << std::endl;
		std::cout << "DownIdentifyFlag = " << server.getDownIdentifyFlag() << std::endl;
		std::cout << std::endl;
		sleep(1);
		// TODO
	}

	return 0;
}

#elif defined TEST2

int main()
{
	RaspiServer server = RaspiServer(TYPE_RTU);
	// std::cout << "Begin" << std::endl;
	server.Init();
	// std::cout << "End" << std::endl;
	// std::cout << "DownPosFlag = " << server.getDownPosFlag() << std::endl;
	while(server.getDownPosFlag() != 1)	
	{
		std::cout << "DownPosFlag = " << server.getDownPosFlag() << std::endl;
		std::cout << std::endl;
		// sleep(1);
	}
	server.setCameraPos(10, 10, 10);
	while(1)
	{

	}
}

#endif