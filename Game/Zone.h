#pragma once
#include "Common.h"
#include "DBSchema.h"
#include "Grid.h"

using db_schema::Map;

class World;
class Hero;
class Monster;

class Zone : std::enable_shared_from_this<Zone>
{
public:
	Zone(const Zone&) = delete;
	Zone& operator=(const Zone&) = delete;

	Zone(const uuid& entity_id, const Map& map_data, World* owner);
	~Zone();

	const uuid& EntityID() const { return entity_id_; }
	int MapID() const { return map_data_.id; }
	const Map& MapData() const { return map_data_; }

	bool Enter(const Ptr<Hero>& pc);
	void Leave(const Ptr<Hero>& pc);

	World* GetWorld() { return owner_; }
	void Update(float delta_time);

private:
	World* owner_;
	
	uuid entity_id_;
	Map map_data_;
	
public:
	// ������ ���� �÷��̾�
	std::map<uuid, Ptr<Hero>> players_;
	// ������ ���� ���� ������Ʈ
	std::map<uuid, Ptr<Monster>> monsters_;
};
