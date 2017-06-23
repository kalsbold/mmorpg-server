#include "stdafx.h"
#include "Monster.h"
#include "Zone.h"

Monster::Monster(const uuid & entity_id)
	: Actor(entity_id)
{}

Monster::~Monster() {}

fb::Offset<PWorld::Monster> Monster::Serialize(fb::FlatBufferBuilder & fbb) const
{
    ProtocolCS::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);
    return PWorld::CreateMonsterDirect(
        fbb,
        boost::uuids::to_string(GetEntityID()).c_str(),
        type_id,
        GetName().c_str(),
        level_,
        max_hp_,
        hp_,
        max_mp_,
        mp_,
        &pos,
        GetRotation()
    );
}
