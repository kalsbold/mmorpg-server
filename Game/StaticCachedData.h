#pragma once
#include <algorithm>
#include "Singleton.h"
#include "DBSchema.h"
#include "MySQL.h"
#include "ServerConfig.h"

namespace mmog {

	using namespace db_schema;

	class MapData : public Singleton<MapData>
	{
	public:
		const std::vector<Ptr<Map>>& Get()
		{
			return map_data_;
		}

		const Ptr<Map> Get(int id)
		{
			auto iter = std::find_if(map_data_.begin(), map_data_.end(),
				[id](const Ptr<Map>& map)
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
				auto db = std::make_shared<MySQLPool>(
					ServerConfig::GetInstance().db_host,
					ServerConfig::GetInstance().db_user,
					ServerConfig::GetInstance().db_password,
					ServerConfig::GetInstance().db_schema,
					ServerConfig::GetInstance().db_connection_pool);

				std::stringstream ss;
				ss << "SELECT * FROM map_tb";

				auto result_set = db->Excute(ss.str());
				while (result_set->next())
				{
					auto map = std::make_shared<Map>();
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
		std::vector<Ptr<Map>> map_data_;
	};

	class CharacterAttributeData : public Singleton<CharacterAttributeData>
	{
	public:
		const std::vector<Ptr<CharacterAttribute>>& Get()
		{
			return data_;
		}

		const Ptr<CharacterAttribute>& Get(ClassType type, int level)
		{
			auto iter = std::find_if(data_.begin(), data_.end(),
				[&](const Ptr<CharacterAttribute>& value)
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
				auto db = std::make_shared<MySQLPool>(
					ServerConfig::GetInstance().db_host,
					ServerConfig::GetInstance().db_user,
					ServerConfig::GetInstance().db_password,
					ServerConfig::GetInstance().db_schema,
					1);

				std::stringstream ss;
				ss << "SELECT * FROM character_attribute_tb";

				auto result_set = db->Excute(ss.str());
				while (result_set->next())
				{
					auto attribute = std::make_shared<CharacterAttribute>();
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
		std::vector<Ptr<CharacterAttribute>> data_;
	};
}

