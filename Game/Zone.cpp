#include "stdafx.h"
#include "Zone.h"
#include "World.h"
#include "GameServer.h"
#include "GameObject.h"
#include "Character.h"
#include "StaticCachedData.h"

Zone::Zone(boost::asio::io_service & ios, World * world, Ptr<db::Map> map_data)
	: strand_(ios)
	, world_(world)
	, map_data_(map_data)
{

}

Zone::~Zone()
{
}

void Zone::Start()
{
	if (update_timer_ != nullptr)
		return;

	update_timer_ = std::make_shared<timer>(strand_.get_io_service());

	time_point start_time = clock::now();
	ScheduleNextUpdate(start_time);
}

void Zone::Stop()
{
	if (update_timer_ == nullptr)
		return;

	update_timer_->cancel();
}

Ptr<GameObject> Zone::GetCharacter(const uuid& uuid)
{
	auto iter = characters_.find(uuid);
	if (iter == characters_.end())
		return nullptr;

	return iter->second;
}

void Zone::EnterCharacter(Ptr<Character> character)
{
	characters_.emplace(character->GetUUID(), character);
	character->SetLocationZone(this);

	for (auto& e : characters_)
	{

	}
}

void Zone::LeaveCharacter(const uuid& uuid)
{
	characters_.erase(uuid);
}

void Zone::Update(double delta_time)
{
	// std::cout << std::this_thread::get_id() << " zone : " << db_map_->name << " update. delta_time : " << delta_time << "\n";
	for (auto& element : characters_)
	{
		element.second->Update(delta_time);
	}
}

void Zone::ScheduleNextUpdate(const time_point & start_time)
{
	update_timer_->expires_at(start_time + timestep);
	update_timer_->async_wait(strand_.wrap([this, start_time](auto& error)
	{
		if (error)
			return;

		HandleNextUpdate(start_time);
	}));
}

void Zone::HandleNextUpdate(const time_point & start_time)
{
	double delta_time = double_seconds(clock::now() - start_time).count();
	ScheduleNextUpdate(clock::now());
	Update(delta_time);
}



