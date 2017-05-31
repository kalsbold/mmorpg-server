#include "stdafx.h"
#include "World.h"
#include "Zone.h"
#include "GameServer.h"
#include "Character.h"
#include "StaticCachedData.h"

World::World()
	: World(std::make_shared<net::IoServiceLoop>(std::thread::hardware_concurrency()))
{
}

World::World(Ptr<net::IoServiceLoop> loop)
	: loop_(loop)
	, strand_(loop->GetIoService())
{
	assert(loop_);
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
	loop_->Stop();
}

Ptr<net::IoServiceLoop> World::GetIosLoop()
{
	return loop_;
}

Zone * World::GetFieldZone(int map_id)
{
	auto iter = std::find_if(field_zones_.begin(), field_zones_.end(), [map_id](const Ptr<Zone>& var)
	{
		return var->GetMapData()->id == map_id;
	});
	if (iter == field_zones_.end())
		return nullptr;

	return (*iter).get();
}

void World::CreateFieldZones()
{
	// 필드존 생성
	auto& map_table = MapTable::GetInstance().GetAll();
	for (auto& map : map_table)
	{
		if (map->type == MapType::FIELD)
		{
			auto zone = std::make_shared<Zone>(loop_->GetIoService(), this, map);
			zone->Start();

			field_zones_.push_back(zone);
		}
	}
}


