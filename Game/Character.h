#pragma once
#include "TypeDef.h"

namespace mmog
{
	using EntityID = boost::uuids::uuid;

	class Actor
	{
	public:
		Actor(const EntityID& entity_id)
			: entity_id_(entity_id)
		{}
		~Actor()
		{}

		virtual void Update(float dt) = 0;

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

}
