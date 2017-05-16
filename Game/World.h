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
	constexpr std::chrono::nanoseconds timestep(50ms);
	using double_seconds = chrono::duration<double>;

	class Zone
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

		Zone(const uuid& uuid, asio::io_service& ios, World* world)
			: uuid_(uuid)
			, strand_(ios)
			, world_(world)
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

		Ptr<GameObject> GetCharacter(uuid uuid)
		{
			auto iter = characters_.find(uuid);
			if (iter == characters_.end())
				return nullptr;

			return iter->second;
		}

		void AddCharacter(uuid uuid, Ptr<mmog::Character> c)
		{
			characters_.insert(make_pair(uuid, c));
		}

		void RemoveCharacter(uuid uuid)
		{
			characters_.erase(uuid);
		}

		const Ptr<db::Map>& GetMap() const
		{
			return map_;
		}

		void SetMap(Ptr<db::Map> map)
		{
			map_ = map;
		}

		void EnterCharacter(Ptr<mmog::Character> character)
		{
			strand_.dispatch([this, character]
			{
				AddCharacter(character->GetUUID(), character);
				character->zone_ = this;

				for (auto& e : characters_)
				{

				}
			});
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

	class World
	{
	public:
		World(Ptr<IoServiceLoop> loop)
			: loop_(loop)
			, strand_(loop->GetIoService())
		{

		}

		~World()
		{
			Stop();
		}

		void Start()
		{
			CreateFieldZones();
		}

		void Stop()
		{

		}

		Ptr<IoServiceLoop> GetIosLoop()
		{
			return loop_;
		}

		bool EnterCharacter(GamePlayer* player, db::Character* db_character)
		{
			// 케릭터 인스턴스 생성
			auto character = mmog::Character::Create();
			character->db_data_ = db_character;
			character->player_ = player;
			// 맵을 찾는다
			auto iter = find_if(field_zones_.begin(), field_zones_.end(), [&](pair<uuid, Ptr<Zone>>& pair)
			{
				return pair.second->GetMap()->id == db_character->map_id;
			});
			if (iter == field_zones_.end())
				return false;
			Ptr<Zone> zone = iter->second;
			zone->EnterCharacter(character);
		}

	private:
		void CreateFieldZones()
		{
			// 필드존 생성
			auto& map_data = MapData::GetInstance().GetAll();
			for (auto& map : map_data)
			{
				if (map->type == MapType::FIELD)
				{
					auto uuid = boost::uuids::random_generator()();
					auto zone = Zone::Create(loop_->GetIoService(), this);
					zone->SetMap(map);
					zone->Start();

					field_zones_.insert(make_pair(uuid, zone));
				}
			}
		}

		Ptr<IoServiceLoop> loop_;
		asio::strand strand_;
		map<uuid, Ptr<Zone>> field_zones_;
		map<uuid, Ptr<Zone>> instance_zones_;
	};

}

