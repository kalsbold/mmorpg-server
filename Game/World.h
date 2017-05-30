#pragma once
#include "Common.h"
#include "DBEntity.h"

class Zone;
class Character;

class World : public std::enable_shared_from_this<World>
{
public:
	World();
	explicit World(Ptr<net::IoServiceLoop> loop);

	~World();

	void Start();

	void Stop();

	Ptr<net::IoServiceLoop> GetIosLoop();

	bool EnterCharacter(Ptr<Character>& character);

private:
	void CreateFieldZones();

	Ptr<net::IoServiceLoop> loop_;
	boost::asio::strand strand_;
	std::map<uuid, Ptr<Zone>> field_zones_;
	std::map<uuid, Ptr<Zone>> instance_zones_;
};
