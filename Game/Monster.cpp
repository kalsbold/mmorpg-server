#include "stdafx.h"
#include "Monster.h"
#include "Zone.h"

Monster::Monster(const uuid & entity_id)
	: Actor(entity_id)
{}

Monster::~Monster() {}

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
        type_id,
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

