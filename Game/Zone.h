#pragma once
#include "Common.h"
#include "DBSchema.h"
#include "Grid.h"

using db_schema::Map;

class World;
class PlayerCharacter;
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

	void Enter(const Ptr<PlayerCharacter>& pc);
	void Leave(const Ptr<PlayerCharacter>& pc);
	
	World* GetWorld() { return owner_; }

	void Update(float delta_time);

private:
	World* owner_;
	
	uuid entity_id_;
	Map map_data_;
	
	// ������ ���� �÷��̾�
	std::map<uuid, Ptr<PlayerCharacter>> players_;
	// ������ ���� ���� ������Ʈ
	std::map<uuid, Ptr<Monster>> monsters_;
};
