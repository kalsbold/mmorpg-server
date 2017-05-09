#pragma once
#include "TypeDef.h"
#include "DBEntity.h"
#include "GameServer.h"

namespace mmog {

	namespace db = db_entity;

	class Zone
	{
	public:
		const uuid& GetUUID() const
		{
			return uuid_;
		}

		const db::Map& GetMapInfo() const
		{
			return map_info_;
		}

		virtual void Update()
		{

		}

		uuid uuid_;
		//asio::strand strand_;
		// ������ ���� �ִ� ���� �÷��̾�
		std::map<SessionID, GamePlayer*> game_users_;
		// �� ����
		db::Map map_info_;
	};

	class Field : public Zone
	{
	public:
		Field() {}
		~Field() {}

		void Update()
		{

		}

	private:
		db::Map map_info_;

	};

	class InstanceDungeon : public Zone
	{
	public:

	};

	class World
	{
	public:
		World() {}
		~World() {}

		void Start() {}
		void Stop() {}

	private:
		//IoServicePool ios_pool_;
	};

}

