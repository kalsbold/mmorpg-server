#include "stdafx.h"
#include "Zone.h"
#include "World.h"
#include "GameServer.h"
#include "GameObject.h"
#include "Character.h"
#include "StaticCachedData.h"

Zone::Zone(const uuid & uuid, boost::asio::io_service & ios, World * world)
	: uuid_(uuid)
	, strand_(ios)
	, world_(world)
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
	AddCharacter(character->GetUUID(), character);
	character->zone_ = this;

	for (auto& e : characters_)
	{

	}
}

void Zone::LeaveCharacter(const uuid& uuid)
{

}

void Zone::Update(double delta_time)
{
	// std::cout << std::this_thread::get_id() << " zone : " << db_map_->name << " update. delta_time : " << delta_time << "\n";
	for (auto& element : characters_)
	{
		element.second->Update(delta_time);
	}
}

void Zone::AddCharacter(const uuid& uuid, Ptr<Character> c)
{
	characters_.insert(make_pair(uuid, c));
}

void Zone::RemoveCharacter(const uuid& uuid)
{
	characters_.erase(uuid);
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



