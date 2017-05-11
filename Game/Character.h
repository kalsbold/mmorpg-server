#pragma once
#include "GameObject.h"
#include "DBEntity.h"

namespace mmog
{
	namespace db = db_entity;

	class Character : public GameObject
	{
	public:
		using GameObject::GameObject;

		static Character* Create()
		{
			return new Character(boost::uuids::random_generator()());
		}

		void SetCharacteristic(db::Character* data)
		{
			db_data_ = data;
		}

		const string& GetName() const
		{
			return db_data_->name;
		}

		void SetOwner(GamePlayer* owner)
		{
			owner_ = owner;
		}

		virtual void Update(float delta_time) override
		{

		}

	private:
		GamePlayer* owner_;
		db::Character* db_data_;
	};

}
