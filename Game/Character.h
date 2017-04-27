#pragma once
#include "TypeDef.h"
#include "DBEntity.h"

namespace mmog
{
	using EntityID = boost::uuids::uuid;

	class Actor
	{
	public:
		Actor()
			: Actor(boost::uuids::random_generator()())
		{}

		explicit Actor(const EntityID& entity_id)
			: entity_id_(entity_id)
		{}
		~Actor()
		{}

		virtual void Update(float delta_time) = 0;

		const EntityID& GetEntityID() const
		{
			return entity_id_;
		}

		void SetPosition(const Vector3& position)
		{
			position_ = position;
		}
		const Vector3& GetPosition() const
		{
			return position_;
		}

		void SetRotation(float y)
		{
			rotation_y_ = y;
		}
		float GetRotation() const
		{
			return rotation_y_;
		}

	private:
		EntityID entity_id_;
		Vector3 position_;
		float rotation_y_;
	};


	namespace db = db_entity;

	class PlayerCharacter : public Actor
	{
	public:
		using Actor::Actor;

		void SetCharacteristic(Ptr<db::Character> value);

		virtual void Update(float delta_time) override
		{

		}
	private:

		Ptr<db::Character> db_character_;


	};

}
