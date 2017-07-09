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

// �±׼���
struct zone_tags
{
	struct entity_id {};
	struct map_id {};
};
// �ε��� Ÿ���� ����
using indices = indexed_by<
	hashed_unique<
		tag<zone_tags::entity_id>, const_mem_fun<Zone, const uuid&, &Zone::EntityID>, boost::hash<boost::uuids::uuid>
	>,
	hashed_unique<
		tag<zone_tags::map_id>, const_mem_fun<Zone, int, &Zone::MapID>
	>
>;
// �����̳� Ÿ�� ����
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
	
	// ����ȭ ����
	template <typename Handler>
	void Dispatch(Handler&& handler)
	{
		strand_.dispatch(std::forward<Handler>(handler));
	}

    template <typename Handler>
    Ptr<timer_type> RunAt(time_point time, Handler&& handler)
    {
        Ptr<timer_type> timer = std::make_shared<timer_type>(strand_.get_io_service(), time);
        timer->async_wait(strand_.wrap([timer, handler = std::forward<Handler>(handler)](const auto& error)
        {
            if (!error)
            {
                if (handler) handler(timer);
            }
            else
            {
                BOOST_LOG_TRIVIAL(info) << error;
            }
        }));
        return timer;
    }

    template <typename Handler>
    Ptr<timer_type> RunAfter(duration duration, Handler&& handler)
    {
        Ptr<timer_type> timer = std::make_shared<timer_type>(strand_.get_io_service(), duration);
        timer->async_wait(strand_.wrap([timer, handler = std::forward<Handler>(handler)](const boost::system::error_code& error)
        {
            if (!error)
            {
                handler(timer);
            }
            else
            {
                BOOST_LOG_TRIVIAL(info) << error;
            }
        }));
        return timer;
    }

	void DoUpdate(float delta_time);

private:

	void CreateFieldZones();
	void CreateZone(const Map& map_data);

	strand strand_;
	ZoneSet zone_set_;
};


