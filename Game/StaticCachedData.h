#pragma once
#include <algorithm>
#include "Common.h"
#include "Singleton.h"
#include "Settings.h"
#include "DBSchema.h"
#include "MySQL.h"

namespace db = db_schema;

class MapTable : public Singleton<MapTable>
{
public:
	const std::vector<db::Map>& GetAll()
	{
		return data_;
	}

	const db::Map* Get(int id)
	{
		auto iter = std::find_if(data_.begin(), data_.end(),
			[id](const db::Map& map)
			{
				return map.id == id;
			});

		return (iter != data_.end()) ? &(*iter) : nullptr;
	}

	static bool Load(Ptr<MySQLPool> db)
	{
		auto& instance = GetInstance();

		try
		{
			instance.data_.clear();

			auto result_set = db->Excute("SELECT * FROM map_tb");
			while (result_set->next())
			{
				db::Map map;
				map.id = result_set->getInt("id");
				map.name = result_set->getString("name").c_str();
				map.width = result_set->getInt("width");
				map.height = result_set->getInt("height");
				map.type = (MapType)result_set->getInt("type");

				instance.data_.push_back(map);
			}
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			return false;
		}

		return true;
	}

private:
	std::vector<db::Map> data_;
};

class CharacterAttributeTable : public Singleton<CharacterAttributeTable>
{
public:
	const std::vector<db::CharacterAttribute>& GetAll()
	{
		return data_;
	}

	const db::CharacterAttribute* Get(ClassType type, int level)
	{
		auto iter = std::find_if(data_.begin(), data_.end(),
			[&](const db::CharacterAttribute& value)
			{
				return (value.class_type == type) && (value.level == level);
			});

		return (iter != data_.end()) ? &(*iter) : nullptr;
	}

	static bool Load(Ptr<MySQLPool> db)
	{
		auto& instance = GetInstance();

		try
		{
			instance.data_.clear();

			auto result_set = db->Excute("SELECT * FROM character_attribute_tb");
			while (result_set->next())
			{
				db::CharacterAttribute attribute;
				attribute.class_type = (ClassType)result_set->getInt("class_type");
				attribute.level = result_set->getInt("level");
				attribute.hp = result_set->getInt("hp");
				attribute.mp = result_set->getInt("mp");
				attribute.att = result_set->getInt("att");
				attribute.def = result_set->getInt("def");
					
				instance.data_.push_back(attribute);
			}

		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			return false;
		}

		return true;
	}

private:
	std::vector<db::CharacterAttribute> data_;
};

