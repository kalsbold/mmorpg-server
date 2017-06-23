#pragma once
#include "Common.h"
#include "Actor.h"
#include "protocol_cs_helper.h"

class Monster : public Actor


{
public:
	Monster(const uuid& entity_id);
	virtual ~Monster();

	virtual void Update(double delta_time) override
	{

	}

    fb::Offset<PWorld::Monster> Serialize(fb::FlatBufferBuilder& fbb) const;

private:

public:
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
