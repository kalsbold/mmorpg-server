#pragma once
#include "TypeDef.h"

namespace mmog
{

	class Hero
	{
	public:
		Hero();
		~Hero();

		int id;
		std::string name;
		ClassType class_type;
		int exp;
		int level;
		int hp;
		int mp;
		int att;
		int def;
		int zone_id;
		float x, z;
	};
}
