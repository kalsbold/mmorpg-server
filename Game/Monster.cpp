#include "stdafx.h"
#include "Monster.h"
#include "Zone.h"

Monster::Monster(const uuid & entity_id)
	: Actor(entity_id)
{}

Monster::~Monster()
{
    Zone* zone = GetZone();
    if (zone != nullptr)
    {
        zone->Exit(GetEntityID());
    }
}

fb::Offset<PCS::World::Actor> Monster::SerializeAsActor(fb::FlatBufferBuilder & fbb) const
{
    //ProtocolCS::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);
    auto mon_offset = SerializeAsMonster(fbb);
    return PCS::World::CreateActor(fbb, PCS::World::ActorType::Monster, mon_offset.Union());
}

fb::Offset<PCS::World::Monster> Monster::SerializeAsMonster(fb::FlatBufferBuilder & fbb) const
{
    //ProtocolCS::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);
    auto mon_offset = PCS::World::CreateMonsterDirect(
        fbb,
        boost::uuids::to_string(GetEntityID()).c_str(),
        uid_,
        type_id_,
        GetName().c_str(),
        level_,
        max_hp_,
        hp_,
        max_mp_,
        mp_,
        //&pos,
        &PCS::Vec3(GetPosition().X, GetPosition().Y, GetPosition().Z),
        GetRotation()
    );

    return mon_offset;
}

int Monster::Uid()
{
    return uid_;
}

int Monster::TypeId()
{
    return type_id_;
}

int Monster::Level()
{
    return level_;
}

int Monster::MaxMp()
{
    return max_mp_;
}

int Monster::Mp()
{
    return mp_;
}

int Monster::Att()
{
    return att_;
}

int Monster::Def()
{
    return def_;
}

bool Monster::IsDead() const
{
    return hp_ <= 0;
}

void Monster::Die()
{
    if (IsDead())
        return;

    if (Hp() != 0)
        Hp(0);

    death_signal_(this);
}

int Monster::MaxHp() const
{
    return max_hp_;
}

void Monster::MaxHp(int max_hp)
{
    if (max_hp < Hp())
    {
        Hp(max_hp);
    }

    max_hp_ = max_hp;
}

int Monster::Hp() const
{
    return hp_;
}

void Monster::Hp(int hp)
{
    if (hp > MaxHp())
        return;

    hp_ = hp;
}

signals2::connection Monster::ConnectDeathSignal(std::function<void(ILivingEntity*)> handler)
{
    return death_signal_.connect(handler);
}

void Monster::Init(const db::Monster & db_data)
{
    uid_ = db_data.uid;
    SetName(db_data.name);
    type_id_ = db_data.type_id;
    level_ = db_data.level;
    MaxHp(db_data.max_hp);
    Hp(db_data.max_hp);
    max_mp_ = db_data.max_mp;
    mp_ = db_data.max_mp;
    att_ = db_data.att;
    def_ = db_data.def;
}

