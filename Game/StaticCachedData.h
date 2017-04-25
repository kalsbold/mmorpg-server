#pragma once
#include "TypeDef.h"
#include "Singleton.h"
#include "DatabaseEntity.h"
#include "MySQL.h"
#include "ServerConfig.h"

namespace mmog {

	class MapData : public Singleton<MapData>
	{
	public:
		const std::vector<Ptr<Map>>& Get()
		{
			return map_data_;
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

	class HeroClassData : public Singleton<HeroClassData>
	{
	public:
		const std::vector<Ptr<HeroClass>>& Get()
		{
			return hero_class_data_;
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
				ss << "SELECT * FROM hero_class_tb";

				auto result_set = db->Excute(ss.str());
				while (result_set->next())
				{
					auto hero_class = std::make_shared<HeroClass>();
					hero_class->class_type = (ClassType)result_set->getInt("class_type");
					hero_class->name = result_set->getString("name").c_str();
					hero_class->hp = result_set->getInt("hp");
					hero_class->mp = result_set->getInt("mp");
					hero_class->att = result_set->getInt("att");
					hero_class->def = result_set->getInt("def");
					hero_class->map_id = result_set->getInt("map_id");
					hero_class->pos.X = result_set->getDouble("pos_x");
					hero_class->pos.Y = result_set->getDouble("pos_y");
					hero_class->pos.Z = result_set->getDouble("pos_z");
					
					instance.hero_class_data_.push_back(hero_class);
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
		std::vector<Ptr<HeroClass>> hero_class_data_;
	};
}

