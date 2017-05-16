#pragma once
#include "GameObject.h"
#include "DBEntity.h"

namespace mmog
{
	namespace db = db_entity;

	class Character : public GameObject
	{
	public:
		friend class Player;
		friend class World;
		friend class Zone;

		using GameObject::GameObject;

		static Ptr<Character> Create()
		{
			return make_shared<Character>(boost::uuids::random_generator()());
		}

		virtual void Update(double delta_time) override
		{

		}

	private:
		GamePlayer* player_;
		db::Character* db_data_;
		Zone* zone_;
	};

}
