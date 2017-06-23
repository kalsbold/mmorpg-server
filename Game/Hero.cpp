#include "stdafx.h"
#include "Hero.h"
#include "Zone.h"

Hero::Hero(const uuid & entity_id, RemoteWorldClient * rc, const db::Hero & db_data)
	: Actor(entity_id)
	, rc_(rc)
{
	InitAttribute(db_data);
}

Hero::~Hero() {}

RemoteWorldClient * Hero::GetRemoteClient()
{
    return rc_;
}

void Hero::Send(const uint8_t * data, size_t size)
{
    rc_->Send(data, size);
}

void Hero::SetCurrentZone(Zone * zone)
{
	Actor::SetCurrentZone(zone);
	if (zone == nullptr) return;

	map_id_ = zone->MapID();
}

void Hero::Move(const Vector3 & position, float rotation, const Vector3 & velocity)
{
    Vector3 t_pos(position);
    t_pos.Y = 0; // Y는 못움직이게

    // 최대 속도보다 빠르게 움직일수 없다. 
    //if (distance(GetPosition(), position) > HERO_MOVE_SPEED * delta_time)
    //    return;
    Zone* zone = GetCurrentZone();
    if (zone == nullptr) return;

    auto& mapData = zone->MapData();
    if (mapData.height > t_pos.Z && 0 < t_pos.Z && mapData.width > t_pos.X && 0 < t_pos.X)
    {
        SetPosition(t_pos);
    }
    SetRotation(rotation);

    t_pos = GetPosition();

    // 주변 플레이어에 통지
    fb::FlatBufferBuilder fbb;
    std::vector<fb::Offset<PWorld::MoveInfo>> list;

    auto offset = PWorld::CreateMoveInfoDirect(fbb,
        boost::uuids::to_string(GetEntityID()).c_str(),
        &PCS::Vec3(t_pos.X, t_pos.Y, t_pos.Z),
        GetRotation(),
        &PCS::Vec3(velocity.X, velocity.Y, velocity.Z));
    list.push_back(offset);

    auto notify = PWorld::CreateNotify_ActionMoveDirect(fbb, &list);
    PCS::FinishMessageRoot(fbb, notify);

    // 지역의 플레이어에게 보낸다.
    for (auto& var : zone->players_)
    {
        var.second->Send(fbb.GetBufferPointer(), fbb.GetSize());
    }
}

void Hero::SetToDB(db::Hero & db_data)
{
    db_data.uid = uid_;
    db_data.exp = exp_;
    db_data.level = level_;
    db_data.max_hp = max_hp_;
    db_data.hp = hp_;
    db_data.max_mp = max_mp_;
    db_data.mp = mp_;
    db_data.att = att_;
    db_data.def = def_;
    db_data.map_id = map_id_;
    db_data.pos = GetPosition();
    db_data.rotation = GetRotation();
}

void Hero::UpdateToDB(const Ptr<MySQLPool>& db)
{
    db::Hero db_data;
    SetToDB(db_data);
    db_data.Update(db);
}

fb::Offset<PWorld::Hero> Hero::Serialize(fb::FlatBufferBuilder & fbb) const
{
    ProtocolCS::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);
    return PWorld::CreateHeroDirect(fbb,
        boost::uuids::to_string(GetEntityID()).c_str(),
        uid_,
        GetName().c_str(),
        (ProtocolCS::ClassType)class_type_,
        exp_,
        level_,
        max_hp_,
        hp_,
        max_mp_,
        mp_,
        att_,
        def_,
        map_id_,
        &pos,
        GetRotation()
    );
}
