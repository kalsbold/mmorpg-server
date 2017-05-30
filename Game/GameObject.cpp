#include "stdafx.h"
#include "GameObject.h"

GameObject::GameObject(const uuid& uuid)
	: uuid_(uuid)
{
}

GameObject::~GameObject()
{
}

const uuid& GameObject::GetUUID() const
{
	return uuid_;
}

void GameObject::SetPosition(const Vector3 & position)
{
	position_ = position;
}

const Vector3 & GameObject::GetPosition() const
{
	return position_;
}

void GameObject::SetRotation(float y)
{
	rotation_ = y;
}

float GameObject::GetRotation() const
{
	return rotation_;
}
