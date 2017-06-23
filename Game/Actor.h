#pragma once
#include "Common.h"
#include "GameObject.h"
#include "DBSchema.h"
#include "protocol_cs_generated.h"

namespace db = db_schema;
namespace fb = flatbuffers;
namespace PCS = ProtocolCS;
namespace PWorld = ProtocolCS::World;

class Zone;

class Actor : public GameObject
{
public:
	Actor(const uuid& entity_id)
        : GameObject(entity_id)
        , current_zone_(nullptr)
    {}

	virtual void SetCurrentZone(Zone* zone)
	{
		current_zone_ = zone;
	}

	virtual Zone* GetCurrentZone()
	{
		return current_zone_;
	}

	const std::string& GetName() const { return name_; }

protected:
	void SetName(const std::string& name) { name_ = name; }

private:
	std::string name_;
	Zone* current_zone_;
};
