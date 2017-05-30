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

bool World::EnterCharacter(Ptr<Character>& character)
{
	// 맵을 찾는다
	auto iter = find_if(field_zones_.begin(), field_zones_.end(), [&](auto& pair)
	{
		return pair.second->GetMap()->id == character->db_data_->map_id;
	});
	if (iter == field_zones_.end())
		return false;

	Ptr<Zone> zone = iter->second;
	zone->EnterCharacter(character);
	return true;
}

void World::CreateFieldZones()
{
	// 필드존 생성
	auto& map_data = MapData::GetInstance().GetAll();
	for (auto& map : map_data)
	{
		if (map->type == MapType::FIELD)
		{
			auto uuid = boost::uuids::random_generator()();
			auto zone = Zone::Create(loop_->GetIoService(), this);
			zone->SetMap(map);
			zone->Start();

			field_zones_.insert(make_pair(uuid, zone));
		}
	}
}


