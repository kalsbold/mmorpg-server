#pragma once
#include "Common.h"
#include "Actor.h"

class Monster : public Actor
{
public:
	Monster(const uuid& entity_id);
	virtual ~Monster();

	virtual void Update(double delta_time) override
	{

	}

	fb::Offset<PWorld::Monster> Serialize(fb::FlatBufferBuilder& fbb) const
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

private:

	int			   type_id;
	int            level_;
	int            max_hp_;
	int            hp_;
	int            max_mp_;
	int            mp_;
	int            att_;
	int            def_;
	int            map_id;
};
