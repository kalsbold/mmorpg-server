#pragma once
#include <algorithm>
#include <unordered_map>
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

class HeroAttributeTable : public Singleton<HeroAttributeTable>
{
public:
	const std::vector<db::HeroAttribute>& GetAll()
	{
		return data_;
	}

	const db::HeroAttribute* Get(ClassType type, int level)
	{
		auto iter = std::find_if(data_.begin(), data_.end(),
			[&](const db::HeroAttribute& value)
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

			auto result_set = db->Excute("SELECT * FROM hero_attribute_tb");
			while (result_set->next())
			{
				db::HeroAttribute attribute;
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
	std::vector<db::HeroAttribute> data_;
};


class MonsterTable : public Singleton<MonsterTable>
{
public:
    const std::unordered_map<int, db::Monster>& GetAll()
    {
        return data_;
    }

    const db::Monster* Get(int uid)
    {
        auto iter = data_.find(uid);

        return (iter != data_.end()) ? &(iter->second) : nullptr;
    }

    static bool Load(Ptr<MySQLPool> db)
    {
        auto& instance = GetInstance();

        try
        {
            instance.data_.clear();

            auto result_set = db->Excute("SELECT * FROM monster_tb");
            while (result_set->next())
            {
                db::Monster row;
                row.uid     = result_set->getInt("uid");
                row.type_id = result_set->getInt("type_id");
                row.name    = result_set->getString("name").c_str();
                row.level   = result_set->getInt("level");
                row.max_hp  = result_set->getInt("max_hp");
                row.max_mp  = result_set->getInt("max_mp");
                row.att     = result_set->getInt("att");
                row.def     = result_set->getInt("def");

                instance.data_.emplace(row.uid, row);
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
    std::unordered_map<int, db::Monster> data_;

};

class MonsterSpawnTable : public Singleton<MonsterSpawnTable>
{
public:
    std::unordered_map<int, db::MonsterSpawn>& GetAll()
    {
        return data_;
    }

    const db::MonsterSpawn* Get(int uid)
    {
        auto iter = data_.find(uid);

        return (iter != data_.end()) ? &(iter->second) : nullptr;
    }

    static bool Load(Ptr<MySQLPool> db)
    {
        auto& instance = GetInstance();

        try
        {
            instance.data_.clear();

            auto result_set = db->Excute("SELECT * FROM monster_spawn_tb");
            while (result_set->next())
            {
                db::MonsterSpawn row;
                row.uid = result_set->getInt("uid");
                row.map_id = result_set->getInt("map_id");
                row.monster_uid = result_set->getInt("monster_uid");
                row.pos = Vector3((float)result_set->getDouble("pos_x"), (float)result_set->getDouble("pos_y"), (float)result_set->getDouble("pos_z"));
                row.interval_s = std::chrono::duration_cast<duration>(std::chrono::seconds(result_set->getInt("interval_s")));

                instance.data_.emplace(row.uid, row);
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
    std::unordered_map<int, db::MonsterSpawn> data_;
};


