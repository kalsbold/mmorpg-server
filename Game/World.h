#pragma once
#include "Common.h"
#include "DBSchema.h"
#include "Zone.h"
#include <boost\multi_index_container.hpp>
#include <boost\multi_index\hashed_index.hpp>
#include <boost\multi_index\member.hpp>
#include <boost\multi_index\mem_fun.hpp>

class Hero;

using namespace boost::multi_index;

// 태그선언
struct zone_tags
{
	struct entity_id {};
	struct map_id {};
};
// 인덱싱 타입을 선언
using indices = indexed_by<
	hashed_unique<
		tag<zone_tags::entity_id>, const_mem_fun<Zone, const uuid&, &Zone::EntityID>, boost::hash<boost::uuids::uuid>
	>,
	hashed_unique<
		tag<zone_tags::map_id>, const_mem_fun<Zone, int, &Zone::MapID>
	>
>;
// 컨테이너 타입 선언
using ZoneSet = boost::multi_index_container<Ptr<Zone>, indices>;

class World : public std::enable_shared_from_this<World>
{
public:
	World(const World&) = delete;
	World& operator=(const World&) = delete;

	World(boost::asio::io_service& ios);
	~World();

	void Start();
	void Stop();
	strand& GetStrand() { return strand_; }
	Zone* FindFieldZone(int map_id);
	
	// 직렬화 실행
	template <typename Handler>
	void Dispatch(Handler&& handler)
	{
		strand_.dispatch(std::forward<Handler>(handler));
	}

	void DoUpdate(float delta_time);

private:

	void CreateFieldZones();
	void CreateZone(const Map& map_data);

	strand strand_;
	ZoneSet zone_set_;
};


