#pragma once
#include <iostream>
#include "Vector3.h"

namespace mmog {

	using Vector3 = AO::Vector3::Vector3;

	enum class ClassType : int
	{
		Knight = 0,
		Archer = 1,
		Mage = 2,
	};
}