#pragma once
#include "Vector3.h"

using namespace AO::Vector3;

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

constexpr float HERO_MOVE_SPEED = 3.0f;