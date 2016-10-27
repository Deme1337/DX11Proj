#pragma once

//cubemap code https://dirkiek.wordpress.com/category/graphics-programming/


#ifndef CUBEMAP_H
#define CUBEMAP_H

#include "DeviceClass.h"

class CubeMap
{
public:
	CubeMap(CDeviceClass *devclass, int mips, int size);
	~CubeMap();

	

private:

};

#endif