#pragma once
#include "TypeDef.h"

namespace mmog {

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

		// 지역에 들어와 있는 게임 플레이어
		std::map<SessionID, Player*> game_users_;
	};


	class World
	{
	public:
		World() {}
		~World() {}

		void Start() {}
		void Stop() {}

	private:
		IoServicePool ios_pool_;
	};

}

