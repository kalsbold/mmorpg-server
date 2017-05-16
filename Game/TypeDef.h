#pragma once
#include <iostream>
#include <gisunnet/gisunnet.h>
#include "Vector3.h"

namespace mmog {

	using namespace gisunnet;

	using AO::Vector3::Vector3;

	enum class ClassType : int
	{
		NONE = 0,
		Knight = 1,
		Archer = 2,
		Mage = 3,
	};

	enum class MapType : int
	{
		NONE = 0,
		FIELD = 1,
		DUNGEON = 2,
	};
}