#pragma once
#include <chrono>
#include <type_traits>
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
class Monster;

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

	Zone(const Zone&) = delete;
	Zone& operator=(const Zone&) = delete;

	Zone(boost::asio::io_service& ios, World* world, Ptr<db::Map> map_data);
	virtual ~Zone();

	boost::asio::strand& GetStrand() { return strand_; }

	void Start();

	void Stop();

	std::map<uuid, Ptr<Character>>& GetCharacters()
	{
		return characters_;
	}

	std::map<uuid, Ptr<Monster>>& GetMonsters()
	{
		return monsters_;
	}

	Ptr<GameObject> GetCharacter(const uuid& uuid);

	const Ptr<db::Map>& GetMapData() const { return map_data_; }

	void EnterCharacter(Ptr<Character> character);

	void LeaveCharacter(const uuid& uuid);

	template <typename Func>
	void Post(Func&& func)
	{
		GetStrand().post(std::forward<Func>(func));
	}

protected:
	virtual void Update(double delta_time);

private:
	void ScheduleNextUpdate(const time_point& start_time);

	void HandleNextUpdate(const time_point& start_time);

	boost::asio::strand strand_;
	Ptr<timer> update_timer_;

	World* world_;
	// 지역에 속한 플레이어
	std::map<uuid, Ptr<Character>> characters_;
	// 지역에 속한 몬스터
	std::map<uuid, Ptr<Monster>> monsters_;
	// 맵 정보
	Ptr<db::Map> map_data_;
};
