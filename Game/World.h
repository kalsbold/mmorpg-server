#pragma once
#include <memory>
#include <gisunnet/gisunnet.h>

using namespace gisunnet;

// ������ �ùķ��̼� �Ǵ� ����
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