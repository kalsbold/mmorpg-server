#pragma once
#include "GameServer.h"
#include "game_message_generated.h"

using namespace gisunnet;
using namespace Game::Protocol;

void OnLoginRequest(const Ptr<Session>& session, NetMessage* net_message)
{
	auto login_request = static_cast<const CS_LoginRequest*>(net_message->message());


}