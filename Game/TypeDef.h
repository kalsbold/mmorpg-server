#pragma once

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
	Field = 1,
	Dungeon = 2,
};

enum class TargetingType : int
{
    NONE = 0,
    Self,
    One,
    Around,
};