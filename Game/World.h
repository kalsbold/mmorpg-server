#pragma once
#include "TypeDef.h"
#include "DBEntity.h"
#include "GameServer.h"

namespace mmog {

	namespace db = db_entity;

	class FrameUpdater
	{
	public:
		void ExcuteFrame()
		{
		}

		std::chrono::milliseconds per_excute_time_;

	private:
		asio::strand strand_;


	};

	class Zone
	{
	public:
		Zone(const uuid& uuid, Ptr<db::Map> map_info)
			: uuid_(uuid)
			, map_info_(map_info)
		{

		}
		virtual ~Zone() {}

		const uuid& GetUUID() const
		{
			return uuid_;
		}

		const db::Map& GetMapInfo() const
		{
			return *map_info_;
		}

	protected:
		virtual void Update(float delta_time)
		{

		}

		uuid uuid_;
		//asio::strand strand_;
		// 지역에 들어와 있는 게임 오브젝트
		std::map<uuid, GameObject*> game_objects_;
		// 맵 정보
		Ptr<db::Map> map_info_;
	};

	class StaticZone : public Zone
	{
	public:
		using Zone::Zone;

		void Update()
		{

		}

	private:

	};

	class InstanceZone : public Zone
	{
	public:
		using Zone::Zone;

	};


	class World
	{
	public:
		World()
		{
		}
		~World()
		{
			Stop();
		}

		void Start() {}
		void Stop()
		{
			loop_pool_->Stop();
		}

	private:


		Ptr<IoServicePool> loop_pool_;
		std::map<uuid, Ptr<StaticZone>> static_zones_;
		std::map<uuid, Ptr<InstanceZone>> instance_zones_;
	};

}

