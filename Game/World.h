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

		// ������ ���� �ִ� ���� �÷��̾�
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

