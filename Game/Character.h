#pragma once
#include "Common.h"
#include "GameObject.h"
#include "DBEntity.h"
#include "protocol_generated.h"

namespace db = db_entity;

class Character : public GameObject
{
public:
	Character(const uuid& uuid, RemoteClient* rc, Ptr<db::Character> db_data)
		: GameObject(uuid)
		, remote_client_(rc)
		, db_data_(db_data)
	{
		InitAttribute();
	}
	virtual ~Character() {}

	RemoteClient* GetRemoteClient()
	{
		return remote_client_;
	}

	void SetLocationZone(Zone* zone)
	{
		location_zone_ = zone;
	}

	Zone* GetLocationZone()
	{
		return location_zone_;
	}

	void UpdateToDB()
	{
		if (!db_data_)
			return;

		db_data_->exp = exp_;
		db_data_->level = level_;
		db_data_->max_hp = max_hp_;
		db_data_->hp = hp_;
		db_data_->max_mp = max_mp_;
		db_data_->mp = mp_;
		db_data_->att = att_;
		db_data_->def = def_;
		db_data_->map_id = map_id_;
		db_data_->pos = GetPosition();
		db_data_->rotation_y = GetRotation();

		db_data_->Update();
	}

	virtual void Update(double delta_time) override
	{

	}

	auto Serialize(flatbuffers::FlatBufferBuilder& fbb)
	{
		auto uuid = fbb.CreateString(boost::uuids::to_string(GetUUID()));
		auto name = fbb.CreateString(name_);
		protocol::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);

		return protocol::world::CreateLocalCharacter(fbb,
			uuid,
			name,
			(protocol::ClassType)class_type_,
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

private:
	void InitAttribute()
	{
		character_id_ = db_data_->id;
		name_         = db_data_->name;
		class_type_   = db_data_->class_type;
		exp_          = db_data_->exp;
		level_        = db_data_->level;
		max_hp_       = db_data_->max_hp;
		hp_           = db_data_->hp;
		max_mp_       = db_data_->max_mp;
		mp_           = db_data_->mp;
		att_          = db_data_->att;
		def_          = db_data_->def;
		map_id_       = db_data_->map_id;
		SetPosition(db_data_->pos);
		SetRotation(db_data_->rotation_y);
	}

	RemoteClient* remote_client_;
	Zone* location_zone_;
	Ptr<db::Character> db_data_;

public:
	int            character_id_;
	std::string    name_;
	ClassType      class_type_;
	int            exp_;
	int            level_;
	int            max_hp_;
	int            hp_;
	int            max_mp_;
	int            mp_;
	int            att_;
	int            def_;
	int            map_id_;
};

class Monster : public GameObject
{
public:
	Monster(const uuid& uuid)
		: GameObject(uuid)
	{}
	virtual ~Monster() {}

	void SetLocationZone(Zone* zone)
	{
		location_zone_ = zone;
	}

	Zone* GetLocationZone()
	{
		return location_zone_;
	}

private:
	RemoteClient* remote_client_;
	Zone* location_zone_;

	std::string    name_;
	ClassType      class_type_;
	int            level_;
	int            max_hp_;
	int            hp_;
	int            max_mp_;
	int            mp_;
	int            att_;
	int            def_;
	int            map_id;
};

