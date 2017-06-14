// EchoClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <iostream>
#include <include/GisunNet.h>
#include <flatbuffers/flatbuffers.h>
#include "monster_generated.h"
#include <boost/locale.hpp>

using namespace std;
using namespace gisun::net;
using namespace MyGame::Sample;

int main()
{
	ClientConfig clientConfig;
	auto client = NetClient::Create(clientConfig);
	client->RegisterNetEventHandler([](const NetEventType& net_event)
	{
		if (net_event == NetEventType::Opened)
		{
			std::cout << "Connect success" << "\n";
		}
		else if (net_event == NetEventType::ConnectFailed)
		{
			std::cout << "Connect failed" << "\n";
		}
		else if (net_event == NetEventType::Closed)
		{
			std::cout << "Connect close" << "\n";
		}
	});

	client->RegisterMessageHandler([](const uint8_t* data, size_t bytes)
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
	});

	client->Connect("127.0.0.1", "8413");
	while (true)
	{
		std::string monster_name;
		std::cin >> monster_name;

		flatbuffers::FlatBufferBuilder builder;


		auto weapon_one_name = builder.CreateString("Sword");
		short weapon_one_damage = std::numeric_limits<short>::max();// 3;
		auto weapon_two_name = builder.CreateString("Axe");
		short weapon_two_damage = std::numeric_limits<short>::max();// 5;
		// Use the `CreateWeapon` shortcut to create Weapons with all the fields set.
		auto sword = CreateWeapon(builder, weapon_one_name, weapon_one_damage);
		auto axe = CreateWeapon(builder, weapon_two_name, weapon_two_damage);

		// Serialize a name for our monster
		auto name = builder.CreateString(monster_name);
		// Create a `vector` representing the inventory of the Orc. Each number
		// could correspond to an item that can be claimed after he is slain.
		uint8_t treasure[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
		auto inventory = builder.CreateVector(treasure, 10);

		// Place the weapons into a `std::vector`, then convert that into a FlatBuffer `vector`.
		std::vector<flatbuffers::Offset<Weapon>> weapons_vector;
		weapons_vector.push_back(sword);
		weapons_vector.push_back(axe);
		auto weapons = builder.CreateVector(weapons_vector);

		// Set his hit points to 300 and his mana to 150.
		int hp = std::numeric_limits<int>::max();//300;
		int mana = std::numeric_limits<int>::max();//150;
		Vec3 pos(1.0f, 2.0f, 3.0f);
		// Finally, create the monster using the `CreateMonster` helper function
		// to set all fields.
		auto monster = CreateMonster(builder, &pos, mana, hp, name,
			inventory, Color_Red, weapons, Equipment_Weapon,
			axe.Union());

		builder.Finish(monster);

		//auto buffer = std::make_shared<Buffer>(str.size());
		//buffer->WriteBytes((uint8_t*)str.data(), str.size());
		std::cout << "size : " << builder.GetSize() << std::endl;
		client->Send(builder.GetBufferPointer(), builder.GetSize());
		//client->Send(builder.GetBufferPointer(), builder.GetSize());
		//client->Send(builder.GetBufferPointer(), builder.GetSize());
	}

    return 0;
}
