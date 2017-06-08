#pragma once
#include "Common.h"
#include "GameObject.h"
#include "DBSchema.h"
#include "protocol_generated.h"
#include "Zone.h"

namespace db = db_schema;

class Actor : public GameObject
{
public:
	using GameObject::GameObject;

	virtual bool IsInZone()
	{
		return location_zone_ != nullptr;
	}

	virtual void SetLocationZone(Zone* zone)
	{
		location_zone_ = zone;
	}

	virtual Zone* GetLocationZone()
	{
		return location_zone_;
	}

protected:
	Zone* location_zone_;
};

class RemoteClient;

class PlayerCharacter : public Actor
{
public:
	PlayerCharacter(const uuid& uuid, RemoteClient* rc, Ptr<db::Character> db_data)
		: Actor(uuid)
		, remote_client_(rc)
		, db_data_(db_data)
	{
		InitAttribute();
	}
	virtual ~PlayerCharacter() {}

	RemoteClient* GetRemoteClient()
	{
		return remote_client_;
	}

	virtual void SetLocationZone(Zone* zone) override
	{
		Actor::SetLocationZone(zone);
		if (zone == nullptr)
			return;

		map_id_ = zone->map_data_.id;
	}

	virtual void Update(double delta_time) override
	{

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

	template<typename T>
	flatbuffers::Offset<T> SerializeAs(flatbuffers::FlatBufferBuilder& fbb) const;

	template<>
	flatbuffers::Offset<protocol::world::PlayerCharacter> SerializeAs<protocol::world::PlayerCharacter>(flatbuffers::FlatBufferBuilder& fbb) const
	{
		auto uuid = fbb.CreateString(boost::uuids::to_string(GetUUID()));
		auto name = fbb.CreateString(name_);
		protocol::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);

		return protocol::world::CreatePlayerCharacter(fbb,
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
			&pos,
			GetRotation()
		);	
	}

	template<>
	flatbuffers::Offset<protocol::world::RemotePC> SerializeAs<protocol::world::RemotePC>(flatbuffers::FlatBufferBuilder& fbb) const
	{
		auto uuid = fbb.CreateString(boost::uuids::to_string(GetUUID()));
		auto name = fbb.CreateString(name_);
		protocol::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);

		return protocol::world::CreateRemotePC(fbb,
			uuid,
			name,
			(protocol::ClassType)class_type_,
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

class Monster : public Actor
{
public:
	Monster(const uuid& uuid)
		: Actor(uuid)
	{}
	virtual ~Monster() {}

	virtual void Update(double delta_time) override
	{

	}

	flatbuffers::Offset<protocol::world::Monster> Serialize(flatbuffers::FlatBufferBuilder& fbb) const
	{
		auto uuid = fbb.CreateString(boost::uuids::to_string(GetUUID()));
		auto name = fbb.CreateString(name_);
		protocol::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);

		return protocol::world::CreateMonster(fbb,
			uuid,
			type_id,
			name,
			level_,
			max_hp_,
			hp_,
			max_mp_,
			mp_,
			&pos,
			GetRotation()
		);
	}
public:
	int			   type_id;
	std::string    name_;
	int            level_;
	int            max_hp_;
	int            hp_;
	int            max_mp_;
	int            mp_;
	int            att_;
	int            def_;
	int            map_id;
};

