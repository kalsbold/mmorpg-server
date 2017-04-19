#pragma once
#include "TypeDef.h"

namespace mmog {
	
	enum MapType
	{
		FIELD = 0,
		DUNGEON = 1,
	};

	struct MapInfo
	{
	public:
		int id;
		string name;
		int width;
		int height;
		MapType type;
	};

	class GameUser;

	class GameZone
	{
	public:

	};

	class Field
	{
	public:
		Field() {}
		~Field() {}

		const uuid& GetUUID() const
		{
			return uuid_;
		}

		const MapInfo& GetMapInfo() const
		{
			return map_info_;
		}

		void Update()
		{

		}

	private:
		asio::strand strand_;
		uuid uuid_;
		MapInfo map_info_;

		// 지역에 들어와 있는 게임 유저
		std::map<SessionID, GameUser*> game_users_;
	};


	class GameWorld
	{
	public:
		GameWorld();
		~GameWorld();

		void Start();
		void Stop();

	private:
		IoServicePool ios_pool_;
	};

	GameWorld::GameWorld()
	{
	}

	GameWorld::~GameWorld()
	{
	}
}

