#pragma once
#include "Common.h"
#include "DBSchema.h"
#include "Grid.h"
#include "ZoneCell.h"
#include "protocol_cs_generated.h"

using db_schema::Map;
using db_schema::MapGate;

class World;
class Actor;
class Hero;
class Monster;
class MonsterSpawner;

constexpr float CELL_SIZE = 10.0f;

class Zone : public Grid<ZoneCell>, std::enable_shared_from_this<Zone>
{
public:
    using GridType = Grid<ZoneCell>;

	Zone(const Zone&) = delete;
	Zone& operator=(const Zone&) = delete;

	Zone(const uuid& entity_id, const Map& map_data, World* owner);
	~Zone();

	const uuid& EntityID() const { return entity_id_; }
	int MapID() const { return map_data_.id; }
	const Map& MapData() const { return map_data_; }

	void Enter(const Ptr<Actor>& actor, const Vector3& position);
	void Exit(const Ptr<Actor>& actor);
    void Exit(const uuid& entity_id);

	World* GetWorld() { return owner_; }
	void Update(float delta_time);

    const MapGate* GetGate(int uid)
    {
        auto iter = map_gates_.find(uid);
        return iter != map_gates_.end() ? iter->second : nullptr;
    }

    Vector3 CheckBoader(const Vector3& position);

    fb::Offset<PCS::World::MapData> Serialize(fb::FlatBufferBuilder& fbb) const;

private:
	World* owner_;
	
	uuid entity_id_;
	Map map_data_;
    std::unordered_map<int, const MapGate*> map_gates_;
	
public:
	// 지역에 속한 Actor
	std::unordered_map<uuid, Ptr<Actor>, std::hash<boost::uuids::uuid>> actors_;
    Ptr<MonsterSpawner> mon_spawner_;
};
