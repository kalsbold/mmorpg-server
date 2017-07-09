#include "stdafx.h"
#include "Zone.h"
#include "RemoteClient.h"
#include "Hero.h"
#include "Monster.h"
#include "World.h"
#include "MonsterSpawner.h"
#include "CachedResources.h"
#include "protocol_cs_helper.h"


Zone::Zone(const uuid & entity_id, const Map & map_data, World * owner)
    : GridType(BoundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(map_data.width, 1.0f, map_data.height)), Vector3(CELL_SIZE, CELL_SIZE, CELL_SIZE))
    , entity_id_(entity_id)
    , map_data_(map_data)
    , owner_(owner)
{
    mon_spawner_ = std::make_shared<MonsterSpawner>(this);
    mon_spawner_->Start();
}

Zone::~Zone()
{}

void Zone::Enter(const Ptr<Actor>& actor, const Vector3& position)
{
    auto r = actors_.emplace(actor->GetEntityID(), actor);

    actor->SetZone(this);
    actor->Spawn(position);
}

void Zone::Exit(const Ptr<Actor>& actor)
{
    actor->ResetInterest();
    actor->SetZone(nullptr);

    actors_.erase(actor->GetEntityID());
}

void Zone::Exit(const uuid & entity_id)
{
    auto iter = actors_.find(entity_id);
    if (iter == actors_.end())
        return;

    auto actor = iter->second;
    actor->ResetInterest();
    actor->SetZone(nullptr);


    actors_.erase(entity_id);
}

void Zone::Update(float delta_time)
{
    for (auto& var : actors_)
    {
        var.second->Update(delta_time);
    }

    mon_spawner_->Update(delta_time);
}
