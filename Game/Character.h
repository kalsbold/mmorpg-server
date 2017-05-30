#pragma once
#include "Common.h"
#include "GameObject.h"
#include "DBEntity.h"

namespace db = db_entity;

class Character : public GameObject
{
public:
	friend class RemoteClient;
	friend class World;
	friend class Zone;

	using GameObject::GameObject;

	static Ptr<Character> Create()
	{
		return std::make_shared<Character>(boost::uuids::random_generator()());
	}

	virtual void Update(double delta_time) override
	{

	}

private:
	RemoteClient* remote_client_;
	db::Character* db_data_;
	Zone* zone_;
};

