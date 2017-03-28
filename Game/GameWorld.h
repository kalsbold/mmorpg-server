#pragma once
#include <memory>
#include <gisunnet/gisunnet.h>
#include "GameSession.h"

using namespace gisunnet;

// ������ �ùķ��̼� �Ǵ� ����
class GameWorld : public std::enable_shared_from_this<GameWorld>
{
public:
	GameWorld()
	{}
	~GameWorld()
	{}

	void Enter(GameUser* session) {}
	void Leave(GameUser* session) {}

	// ������ ������Ʈ
	void Update() {}

private:
	// ���ӿ� ���� ������
	std::vector<GameUser*> game_sessions;

};
