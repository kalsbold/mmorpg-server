#pragma once

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>

using entity_id = boost::uuids::uuid;
/*
class ComponentBase;
class GameObject
{
public:
	GameObject(entity_id entityID);
	~GameObject();

	void Update(float elapsedTime);

	entity_id GetEntityID() const;

	void SetPosition(const Vector3& position);
	const Vector3& GetPosition() const;

	void SetOrientation(const Quaternion& orientation);
	const Quaternion& GetOrientation() const;

	bool InsertComponent(ComponentBase* pComponent);
	ComponentBase* GetComponent(const component_id& componentID);
	const ComponentBase*GetComponent(const component_id& componentID) const;

private:
	entity_id m_entityID;

	Vector3 m_position;
	Quaternion m_orientation;

	boost::unorderd_map<component_id, ComponentBase*> m_components;
};
*/