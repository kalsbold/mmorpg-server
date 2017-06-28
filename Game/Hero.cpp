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

void Hero::SetZone(Zone * zone)
{
	Actor::SetZone(zone);
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
    if (GetZone() == nullptr) return;

    auto& mapData = GetZone()->MapData();
    if (mapData.height > t_pos.Z && 0 < t_pos.Z && mapData.width > t_pos.X && 0 < t_pos.X)
    {
        SetPosition(t_pos);
    }
    SetRotation(rotation);
    if (poistion_update_handler)
        poistion_update_handler(GetPosition());

    UpdateInterest();

    PCS::World::MoveInfoT move_info;
    move_info.entity_id = uuids::to_string(GetEntityID());
    move_info.position = std::make_unique<PCS::Vec3>(GetPosition().X, GetPosition().Y, GetPosition().Z);
    move_info.rotation = GetRotation();
    move_info.velocity = std::make_unique<PCS::Vec3>(velocity.X, velocity.Y, velocity.Z);

    PCS::World::Notify_UpdateT update_msg;
    update_msg.update_data.Set(std::move(move_info));
    // 통지
    PublishActorUpdate(&update_msg);
}

inline fb::Offset<PCS::World::Actor> Hero::SerializeAsActor(fb::FlatBufferBuilder & fbb) const
{
    //ProtocolCS::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);
    auto hero_offset = SerializeAsHero(fbb);
    return PCS::World::CreateActor(fbb, PCS::World::ActorType::Hero, hero_offset.Union());
}

inline fb::Offset<PCS::World::Hero> Hero::SerializeAsHero(fb::FlatBufferBuilder & fbb) const
{
    //ProtocolCS::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);
    auto hero_offset = PCS::World::CreateHeroDirect(fbb,
        boost::uuids::to_string(GetEntityID()).c_str(),
        uid_,
        GetName().c_str(),
        (PCS::ClassType)class_type_,
        exp_,
        level_,
        max_hp_,
        hp_,
        max_mp_,
        mp_,
        att_,
        def_,
        map_id_,
        //&pos,
        &PCS::Vec3(GetPosition().X, GetPosition().Y, GetPosition().Z),
        GetRotation()
    );

    return hero_offset;
}

inline void Hero::InitAttribute(const db::Hero & db_data)
{
    uid_ = db_data.uid;
    SetName(db_data.name);
    class_type_ = db_data.class_type;
    exp_ = db_data.exp;
    level_ = db_data.level;
    max_hp_ = db_data.max_hp;
    hp_ = db_data.hp;
    max_mp_ = db_data.max_mp;
    mp_ = db_data.mp;
    att_ = db_data.att;
    def_ = db_data.def;
    map_id_ = db_data.map_id;
    SetPosition(db_data.pos);
    SetRotation(db_data.rotation);
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

