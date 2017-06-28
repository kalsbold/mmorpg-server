#include "stdafx.h"
#include "Zone.h"
#include "RemoteClient.h"
#include "Hero.h"
#include "Monster.h"
#include "World.h"
#include "CachedResources.h"
#include "protocol_cs_helper.h"


Zone::Zone(const uuid & entity_id, const Map & map_data, World * owner)
    : GridType(BoundingBox(Vector3(0.0f, 0.0f, 0.0f), Vector3(map_data.width, 1.0f, map_data.height)), Vector3(CELL_SIZE, CELL_SIZE, CELL_SIZE))
    , entity_id_(entity_id)
    , map_data_(map_data)
    , owner_(owner)
{}

Zone::~Zone()
{}

bool Zone::Enter(const Ptr<Hero>& pc, const Vector3& position)
{
    auto r = players_.emplace(pc->GetEntityID(), pc);
    if (!r.second) return false;

    pc->SetZone(this);
    pc->SetPosition(position);
    pc->UpdateInterest();
}

void Zone::Exit(const Ptr<Hero>& pc)
{
    size_t r = players_.erase(pc->GetEntityID());
    if (r == 0) return;

    pc->SetZone(nullptr);
    pc->ResetInterest();
}

void Zone::Update(float delta_time)
{
    for (auto& var : players_)
    {
        var.second->Update(delta_time);
    }
    for (auto& var : monsters_)
    {
        var.second->Update(delta_time);
    }
}
