#include "stdafx.h"
#include "World.h"
#include "CachedResources.h"


World::World(boost::asio::io_service & ios)
	: strand_(ios)
{
}

World::~World()
{
	Stop();
}

void World::Start()
{
	CreateFieldZones();
}

void World::Stop()
{

}

Zone * World::FindFieldZone(int map_id)
{
	auto& indexer = zone_set_.get<zone_tags::map_id>();
	auto iter = indexer.find(map_id);
	if (iter == indexer.end())
		return nullptr;

	return iter->get();
}

void World::DoUpdate(float delta_time)
{
	for(auto& var : zone_set_)
	{
		var->Update(delta_time);
	}
}

void World::CreateFieldZones()
{
	// 필드존 생성
	auto& map_table = MapTable::GetInstance().GetAll();
	for (auto& map_data : map_table)
	{
		if (map_data.type == MapType::Field)
			CreateZone(map_data);
	}
}

void World::CreateZone(const Map & map_data)
{
	auto zone = std::make_shared<Zone>(random_generator()(), map_data, this);
	zone_set_.insert(zone);
}


