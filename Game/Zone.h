#pragma once
#include "Common.h"
#include "DBSchema.h"
#include "Grid.h"
#include "ZoneCell.h"
#include "protocol_cs_generated.h"

using db_schema::Map;

class World;
class Actor;
class Hero;
class Monster;

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

	bool Enter(const Ptr<Hero>& pc, const Vector3& position);
	void Exit(const Ptr<Hero>& pc);

	World* GetWorld() { return owner_; }
	void Update(float delta_time);

private:
	World* owner_;
	
	uuid entity_id_;
	Map map_data_;
	
public:
	// 지역에 속한 플레이어
	std::map<uuid, Ptr<Hero>> players_;
	// 지역에 속한 게임 오브젝트
	std::map<uuid, Ptr<Monster>> monsters_;
};
