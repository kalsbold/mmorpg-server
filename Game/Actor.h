#pragma once
#include "Common.h"
#include "GameObject.h"
#include "DBSchema.h"
#include "protocol_cs_generated.h"
#include <boost/signals2.hpp>

namespace db = db_schema;
namespace fb = flatbuffers;
namespace PCS = ProtocolCS;

class Zone;
class ZoneCell;

class Actor : public GameObject
{
public:
	Actor(const uuid& entity_id)
        : GameObject(entity_id)
        , zone_(nullptr)
        , current_cell_(nullptr)
    {}

    ~Actor()
    {
        ResetInterest();
    }

	const std::string& GetName() const { return name_; }

    virtual void SetZone(Zone* zone)
    {
        if (zone_ == zone)
            return;

        zone_ = zone;
    }

    Zone* GetZone()
    {
        return zone_;
    }

    ZoneCell* GetCurrentCell()
    {
        return current_cell_;
    }

    void UpdateInterest();

    void ResetInterest();

    void PublishActorUpdate(PCS::World::Notify_UpdateT* message);

    virtual fb::Offset<PCS::World::Actor> SerializeAsActor(fb::FlatBufferBuilder& fbb) const
    {
        return 0;
    }

protected:
	void SetName(const std::string& name) { name_ = name; }

private:
	std::string name_;
	Zone* zone_;
    ZoneCell* current_cell_;
    std::vector<signals2::connection> cell_connections_;
};
