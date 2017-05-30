#pragma once
#include <chrono>
#include "Common.h"
#include "DBEntity.h"

using namespace std::chrono_literals;
namespace db = db_entity;

// 50fps
constexpr std::chrono::nanoseconds timestep(50ms);
using double_seconds = std::chrono::duration<double>;

class World;
class GameObject;
class Character;

class Zone : std::enable_shared_from_this<Zone>
{
public:
	// The clock type.
	using clock = std::chrono::high_resolution_clock;
	// The duration type of the clock.
	using duration = clock::duration;
	// The time point type of the clock.
	using time_point = clock::time_point;
	// timer type
	using timer = boost::asio::high_resolution_timer;

	static Ptr<Zone> Create(boost::asio::io_service& ios, World* world)
	{
		return std::make_shared<Zone>(boost::uuids::random_generator()(), ios, world);
	}

	Zone(const uuid& uuid, boost::asio::io_service& ios, World* world);
	virtual ~Zone();

	const uuid& GetUUID() const { return uuid_; }
	boost::asio::strand& GetStrand() { return strand_; }

	void Start();

	void Stop();

	Ptr<GameObject> GetCharacter(const uuid& uuid);

	const Ptr<db::Map>& GetMap() const { return db_map_; }
	void SetMap(Ptr<db::Map> map) { db_map_ = map; }

	void EnterCharacter(Ptr<Character> character);

	void LeaveCharacter(const uuid& uuid);

protected:
	virtual void Update(double delta_time);

	void Broadcast()
	{
		for (auto& e : characters_)
		{

		}
	}

private:
	void AddCharacter(const uuid& uuid, Ptr<Character> c);
	void RemoveCharacter(const uuid& uuid);

	void ScheduleNextUpdate(const time_point& start_time);

	void HandleNextUpdate(const time_point& start_time);

	boost::asio::strand strand_;
	Ptr<timer> update_timer_;
	uuid uuid_;

	World* world_;
	// 지역에 속한 플레이어
	std::map<uuid, Ptr<Character>> characters_;
	// 맵 정보
	Ptr<db::Map> db_map_;
};
