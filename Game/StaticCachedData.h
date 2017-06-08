#pragma once
#include <algorithm>
#include "Common.h"
#include "Singleton.h"
#include "ServerSettings.h"
#include "DBSchema.h"
#include "MySQL.h"

namespace db = db_schema;

class MapTable : public Singleton<MapTable>
{
public:
	const std::vector<Ptr<db::Map>>& GetAll()
	{
		return map_data_;
	}

	const Ptr<db::Map> Get(int id)
	{
		auto iter = std::find_if(map_data_.begin(), map_data_.end(),
			[id](const Ptr<db::Map>& map)
			{
				return map->id == id;
			});

		return (iter != map_data_.end()) ? *iter : nullptr;
	}

	static bool Load()
	{
		auto& instance = GetInstance();

		try
		{
			instance.map_data_.clear();

			auto db = std::make_shared<MySQLPool>(
				ServerSettings::GetInstance().db_host,
				ServerSettings::GetInstance().db_user,
				ServerSettings::GetInstance().db_password,
				ServerSettings::GetInstance().db_schema,
				ServerSettings::GetInstance().db_connection_pool);

			auto result_set = db->Excute("SELECT * FROM map_tb");
			while (result_set->next())
			{
				auto map = std::make_shared<db::Map>();
				map->id = result_set->getInt("id");
				map->name = result_set->getString("name").c_str();
				map->width = result_set->getInt("width");
				map->height = result_set->getInt("height");
				map->type = (MapType)result_set->getInt("type");

				instance.map_data_.push_back(map);
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
	std::vector<Ptr<db::Map>> map_data_;
};

class CharacterAttributeTable : public Singleton<CharacterAttributeTable>
{
public:
	const std::vector<Ptr<db::CharacterAttribute>>& GetAll()
	{
		return data_;
	}

	const Ptr<db::CharacterAttribute> Get(ClassType type, int level)
	{
		auto iter = std::find_if(data_.begin(), data_.end(),
			[&](const Ptr<db::CharacterAttribute>& value)
			{
				return (value->class_type == type) && (value->level == level);
			});

		return (iter != data_.end()) ? *iter : nullptr;
	}

	static bool Load()
	{
		auto& instance = GetInstance();

		try
		{
			instance.data_.clear();

			auto db = std::make_shared<MySQLPool>(
				ServerSettings::GetInstance().db_host,
				ServerSettings::GetInstance().db_user,
				ServerSettings::GetInstance().db_password,
				ServerSettings::GetInstance().db_schema,
				1);

			auto result_set = db->Excute("SELECT * FROM character_attribute_tb");
			while (result_set->next())
			{
				auto attribute = std::make_shared<db::CharacterAttribute>();
				attribute->class_type = (ClassType)result_set->getInt("class_type");
				attribute->level = result_set->getInt("level");
				attribute->hp = result_set->getInt("hp");
				attribute->mp = result_set->getInt("mp");
				attribute->att = result_set->getInt("att");
				attribute->def = result_set->getInt("def");
					
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
	std::vector<Ptr<db::CharacterAttribute>> data_;
};

