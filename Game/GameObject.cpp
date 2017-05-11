#include "stdafx.h"
#include "GameObject.h"

namespace mmog {

GameObject::GameObject(uuid uuid)
	: uuid_(uuid)
{
}

GameObject::~GameObject()
{
}

inline uuid GameObject::GetUUID() const
{
	return uuid_;
}

inline void GameObject::SetPosition(const Vector3 & position)
{
	position_ = position;
}

inline const Vector3 & GameObject::GetPosition() const
{
	return position_;
}

inline void GameObject::SetRotation(float y)
{
	rotation_ = y;
}

inline float GameObject::GetRotation() const
{
	return rotation_;
}

}