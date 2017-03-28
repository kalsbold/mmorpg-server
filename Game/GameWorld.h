#pragma once
#include <memory>
#include <gisunnet/gisunnet.h>
#include "GameSession.h"

using namespace gisunnet;

// 게임이 시뮬레이션 되는 단위
class GameWorld : public std::enable_shared_from_this<GameWorld>
{
public:
	GameWorld()
	{}
	~GameWorld()
	{}

	void Enter(GameUser* session) {}
	void Leave(GameUser* session) {}

	// 프레임 업데이트
	void Update() {}

private:
	// 게임에 들어온 유저들
	std::vector<GameUser*> game_sessions;

};
