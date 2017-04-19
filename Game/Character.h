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

	enum class ClassType : int
	{
		Knight = 0,
		Archer = 1,
		Mage = 2,
	};

	struct CharacterData
	{
	public:
		int id; // database id
		std::string name;
		ClassType class_type;
		int exp;
		int level;
		int hp;
		int mp;
		int att;
		int def;
		int map_id;
		Vector3 pos;
		float rotation_y;
		Vector3 direction;
		float speed;

	};
	class PlayerCharacter : public Actor
	{
	public:
		PlayerCharacter(const EntityID& entity_id)
			: Actor(entity_id)
		{

		}
		~PlayerCharacter()
		{
		}

		void Update(float dt) override
		{

		}

		void SetCharacterData(const CharacterData& data)
		{
		}
	private:
		int character_id; // user_character_tb.id
		std::string name_;
		ClassType class_type_;
		int exp_;
		int level_;
		int hp_;
		int mp_;
		int att_;
		int def_;
	};
}
