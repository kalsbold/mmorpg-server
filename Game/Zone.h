#pragma once
#include <boost/asio/high_resolution_timer.hpp>
#include "TypeDef.h"
#include "DBEntity.h"
#include "GameServer.h"
#include "GameObject.h"
#include "Character.h"
#include "StaticCachedData.h"

namespace mmog {

	using namespace std;
	using namespace chrono;
	namespace db = db_entity;

	// 50fps
	constexpr chrono::nanoseconds timestep(50ms);
	using double_seconds = chrono::duration<double>;

	class World;

	class Zone : enable_shared_from_this<Zone>
	{
	public:
		// The clock type.
		using clock = chrono::high_resolution_clock;
		// The duration type of the clock.
		using duration = clock::duration;
		// The time point type of the clock.
		using time_point = clock::time_point;
		// timer type
		using timer = asio::high_resolution_timer;

		static Ptr<Zone> Create(asio::io_service& ios, World* world)
		{
			return make_shared<Zone>(boost::uuids::random_generator()(), ios, world);
		}

		Zone(const uuid& uuid, asio::io_service& ios, World* world);
		virtual ~Zone();

		const uuid& GetUUID() const { return uuid_; }
		asio::strand& GetStrand() { return strand_; }

		void Start()
		{
			if (update_timer_ != nullptr)
				return;

			update_timer_ = make_shared<timer>(strand_.get_io_service());

			time_point start_time = clock::now();
			ScheduleNextUpdate(start_time);
		}

		void Stop()
		{
			if (update_timer_ == nullptr)
				return;

			update_timer_->cancel();
		}

		Ptr<GameObject> GetCharacter(uuid uuid);

		const Ptr<db::Map>& GetMap() const { return map_; }
		void SetMap(Ptr<db::Map> map) { map_ = map; }

		void EnterCharacter(Ptr<mmog::Character> character)
		{
			AddCharacter(character->GetUUID(), character);
			character->zone_ = this;

			for (auto& e : characters_)
			{

			}
		}

		void LeaveCharacter(uuid uuid)
		{

		}

	protected:
		virtual void Update(double delta_time)
		{
			cout << this_thread::get_id() << " zone : " << map_->name << " update. delta_time : " << delta_time << "\n";
			for (auto& element : characters_)
			{
				element.second->Update(delta_time);
			}
		}

		void Broadcast()
		{
			for (auto& e : characters_)
			{

			}
		}

	private:
		void AddCharacter(uuid uuid, Ptr<mmog::Character> c);
		void RemoveCharacter(uuid uuid);

		void ScheduleNextUpdate(const time_point& start_time)
		{
			update_timer_->expires_at(start_time + timestep);
			update_timer_->async_wait(strand_.wrap([this, start_time](auto& error)
			{
				if (error)
					return;

				HandleNextUpdate(start_time);
			}));
		}

		void HandleNextUpdate(const time_point& start_time)
		{
			double delta_time = double_seconds(clock::now() - start_time).count();
			ScheduleNextUpdate(clock::now());
			Update(delta_time);
		}

		asio::strand strand_;
		Ptr<timer> update_timer_;
		uuid uuid_;

		World* world_;
		// 지역에 속한 플레이어
		map<uuid, Ptr<mmog::Character>> characters_;
		// 맵 정보
		Ptr<db::Map> map_;
	};

}
