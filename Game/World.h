#pragma once
#include "Common.h"

class Zone;
class Character;

class World : public std::enable_shared_from_this<World>
{
public:
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	World();
	explicit World(Ptr<net::IoServiceLoop> loop);

	~World();

	void Start();

	void Stop();

	Ptr<net::IoServiceLoop> GetIosLoop();

	Zone* GetFieldZone(int map_id);

private:
	void CreateFieldZones();

	Ptr<net::IoServiceLoop> loop_;
	boost::asio::strand strand_;
	std::vector<Ptr<Zone>> field_zones_;
	//std::map<uuid, Ptr<Zone>> instance_zones_;
};
