// EchoServer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <gisunnet/network/NetServer.h>
#include <gisunnet/network/IoServicePool.h>
#include <flatbuffers/flatbuffers.h>
#include "monster_generated.h"
#include <boost/locale.hpp>

using namespace gisunnet;
using namespace MyGame::Sample;

int main()
{
	Configuration config;
	config.max_session_count = 5;
	auto server = NetServer::Create(config);

	server->RegisterSessionOpenedHandler([](auto& session)
	{
		std::cout << "Connect session id :" << session->ID() << "\n";
		string ip;
		uint16_t port;
		session->GetRemoteEndpoint(ip, port);
		std::cout << "Connect from :" << ip << ":" << port << "\n";
	});

	server->RegisterSessionClosedHandler([](auto& session, auto& reason)
	{
		std::cout << "Close session id :" << session->ID() << "\n";
	});

	server->RegisterMessageHandler([](auto& session, const uint8_t* data, size_t bytes)
	{
		auto monster = GetMonster(data);

		auto hp = monster->hp();
		auto mana = monster->mana();
		auto name = monster->name()->c_str();
		auto pos = monster->pos();
		auto x = pos->x();
		auto y = pos->y();
		auto z = pos->z();
		auto inv = monster->inventory(); // A pointer to a `flatbuffers::Vector<>`.
		auto inv_len = inv->Length();
		auto third_item = inv->Get(2);
		auto weapons = monster->weapons(); // A pointer to a `flatbuffers::Vector<>`.
		auto weapon_len = weapons->Length();
		auto second_weapon_name = weapons->Get(1)->name()->str();
		auto second_weapon_damage = weapons->Get(1)->damage();
		auto union_type = monster->equipped_type();
		if (union_type == Equipment_Weapon) {
			auto weapon = static_cast<const Weapon*>(monster->equipped()); // Requires `static_cast`
																		   // to type `const Weapon*`.
			auto weapon_name = weapon->name()->str(); // "Axe"
			auto weapon_damage = weapon->damage();    // 5
		}

		//std::cout << "Monster name : " << boost::locale::conv::from_utf<char>(name, "EUC-KR") << "\n";
		std::cout << "Monster name : " << name << "\n";
		session->Send(data, bytes);
	});

	server->Start(8413);

	std::string input;
	std::getline(std::cin, input, '\n');
	
	server->Stop();
	auto ios_pool = server->GetIoServicePool();
	ios_pool->Stop();
	ios_pool->Wait();

    return 0;
}

