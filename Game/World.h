#pragma once
#include <memory>
#include <gisunnet/gisunnet.h>

using namespace gisunnet;

// 게임이 시뮬레이션 되는 단위
class World : public std::enable_shared_from_this<World>
{
public:
	World();
	~World();

private:

};

World::World()
{
}

World::~World()
{
}