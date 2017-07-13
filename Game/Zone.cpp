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
    auto& map_gate_table = MapGateTable::GetInstance().GetAll();
    std::for_each(map_gate_table.begin(), map_gate_table.end(), [this](const MapGate& value)
    {
        if (value.map_id == MapID())
        {
            map_gates_.emplace(value.uid, &value);
        }
    });

    mon_spawner_ = std::make_shared<MonsterSpawner>(this);
    mon_spawner_->Start();
}

Zone::~Zone()
{}

void Zone::Enter(const Ptr<Actor>& actor, const Vector3& position)
{
    auto r = actors_.emplace(actor->GetEntityID(), actor);

    actor->SetZone(this);
    actor->Spawn(CheckBoader(position));
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

Vector3 Zone::CheckBoader(const Vector3 & position)
{
    const auto& area = Area();
    Vector3 result = position;

    if (position.X < area.min.X)
    {
        result.X = area.min.X;
    }
    if (position.X > area.max.X)
    {
        result.X = area.max.X;
    }
    if (position.Y < area.min.Y)
    {
        result.Y = area.min.Y;
    }
    if (position.Y > area.max.Y)
    {
        result.Y = area.max.Y;
    }
    if (position.Z < area.min.Z)
    {
        result.Z = area.min.Z;
    }
    if (position.Z > area.max.Z)
    {
        result.Z = area.max.Z;
    }

    return result;
}

fb::Offset<PCS::World::MapData> Zone::Serialize(fb::FlatBufferBuilder & fbb) const
{
    std::vector<fb::Offset<PCS::World::GateInfo>> map_gates;
    for (auto& e : map_gates_)
    {
        auto gate = e.second;
        auto offset = PCS::World::CreateGateInfo(fbb,
            gate->uid,
            &PCS::Vec3(gate->pos.X, gate->pos.Y, gate->pos.Z));

        map_gates.emplace_back(offset);
    }

    return PCS::World::CreateMapDataDirect(fbb,
        boost::uuids::to_string(EntityID()).c_str(),
        MapID(),
        (PCS::MapType)map_data_.type,
        &map_gates);
}
