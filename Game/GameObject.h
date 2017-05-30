#pragma once
#include "Common.h"

class GameObject
{
public:
	GameObject(const uuid& uuid);
	virtual ~GameObject();

	virtual void Update(double delta_time) = 0;

	const uuid& GetUUID() const;
	void SetPosition(const Vector3& position);
	const Vector3& GetPosition() const;
	void SetRotation(float y);
	float GetRotation() const;

private:
	uuid uuid_;
	Vector3 position_;
	float rotation_;
};
