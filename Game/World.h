#pragma once
#include "TypeDef.h"
#include "DBEntity.h"
#include "GameServer.h"
#include "GameObject.h"

namespace mmog {

	using namespace std;
	using namespace chrono;
	namespace db = db_entity;

	class Zone
	{
	public:
		/// The clock type.
		using Clock = chrono::steady_clock;
		/// The duration type of the clock.
		using duration = Clock::duration;
		/// The time point type of the clock.
		using time_point = Clock::time_point;

		using double_seconds = chrono::duration<double>;

		Zone(const uuid& uuid, asio::io_service& ios)
			: uuid_(uuid)
			, strand_(ios)
		{

		}
		virtual ~Zone() {}

		const uuid& GetUUID() const
		{
			return uuid_;
		}

		asio::strand& GetStrand()
		{
			return strand_;
		}

		void Start(double update_fps)
		{
			if (update_timer_)
				return;

			update_timer_ = make_shared<asio::steady_timer>(strand_.get_io_service());

			double_seconds sec_per_frame(1.0 / update_fps);
			time_point start_time = Clock::now();
			update_timer_->expires_from_now(chrono::duration_cast<duration>(sec_per_frame));
			update_timer_->async_wait(strand_.wrap([this, start_time] { HandleUpdate(start_time); }));
		}

		void HandleUpdate(time_point start_time)
		{
			double delta_time = double_seconds(start_time - Clock::now()).count;
			start_time = Clock::now();

			Update(delta_time);

			update_timer_->expires_from_now(milli_per_frame);
			update_timer_->async_wait(strand_.wrap([this, start_time] { DoUpdate(start_time); }))
		}

		GameObject* GetGameObject(uuid uuid)
		{

		}

		void AddGameObject(uuid uuid, GameObject* go)
		{

		}

		void RemoveGameObject(uuid uuid)
		{

		}

	protected:
		virtual void Update(float delta_time) = 0;
		
		asio::strand strand_;
		Ptr<asio::steady_timer> update_timer_;	
		uuid uuid_;
		// 지역에 속한 게임 오브젝트
		std::map<uuid, GameObject*> game_objects_;
	};

	class StaticZone : public Zone
	{
	public:
		using Zone::Zone;

		const Ptr<db::Map>& GetMap() const
		{
			return map_;
		}

		void SetMap(Ptr<db::Map> map)
		{
			map_ = map;
		}

	protected:
		void Update(float delta_time) override
		{
			UpdateGameObjects(delta_time);
		}

		void UpdateGameObjects(float delta_time)
		{
			for (auto& element : game_objects_)
			{
				GameObject* go = element.second;
				go->Update(delta_time);
			}
		}

		// 맵 정보
		Ptr<db::Map> map_;
	};

	class InstanceZone : public Zone
	{
	public:
		using Zone::Zone;

	};


	class World
	{
	public:
		World(Ptr<IoServiceLoop> loop)
			: loop_(loop)
		{

		}
		~World()
		{
			Stop();
		}

		void Start() {}
		void Stop()
		{
			loop_->Stop();
		}

	private:
		void Update(float delta_time)
		{

		}

		Ptr<IoServiceLoop> loop_;
		std::map<uuid, Ptr<StaticZone>> static_zones_;
		std::map<uuid, Ptr<InstanceZone>> instance_zones_;
	};

}

