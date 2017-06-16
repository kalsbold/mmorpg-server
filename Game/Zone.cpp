#include "stdafx.h"
#include "Zone.h"
#include "RemoteClient.h"
#include "PlayerCharacter.h"
#include "Monster.h"
#include "World.h"
#include "StaticCachedData.h"
#include "protocol_cs_helper.h"


Zone::Zone(const uuid & entity_id, const Map & map_data, World * owner)
	: entity_id_(entity_id), map_data_(map_data), owner_(owner)
{}

Zone::~Zone()
{}

void Zone::Enter(const Ptr<PlayerCharacter>& pc)
{
	auto r = players_.emplace(pc->GetEntityID(), pc);
	if (!r.second) return;

	pc->SetLocationZone(this);

	// TO DO : 공간 분할을 고려할것(격자, 쿼드트리 등)
	// 주변 플레이어에 입장 통지
	fb::FlatBufferBuilder fbb;
	auto remote_pc = pc->SerializeAs<PWorld::RemotePC>(fbb);
	std::vector<fb::Offset<PWorld::RemotePC>> remote_pc_list;
	remote_pc_list.push_back(remote_pc);
	auto notify_appear_actor = PWorld::CreateNotify_AppearActorDirect(fbb, &remote_pc_list);

	// 여러 명에게 보내기 때문에 공유버퍼 사용.
	auto buffer = PCS::MakeBuffer(fbb, notify_appear_actor);
	// 지역의 플레이어에게 등장 메시지를 보낸다.
	for (auto& var : players_)
	{
		var.second->Send(buffer);
	}

	// 자신에게 주변 오브젝트 정보를 보낸다.
	fbb.Clear();
	remote_pc_list.clear();
	std::vector<fb::Offset<PWorld::Monster>> monster_vector;

	for (auto& var : players_)
	{
		auto remote_pc = var.second->SerializeAs<PWorld::RemotePC>(fbb);
		remote_pc_list.push_back(remote_pc);
	}
	for (auto& var : monsters_)
	{
		auto monster = var.second->Serialize(fbb);
		monster_vector.push_back(monster);
	}

	notify_appear_actor = PWorld::CreateNotify_AppearActorDirect(fbb, &remote_pc_list, &monster_vector);
	PCS::Send(*(pc->GetRemoteClient()), fbb, notify_appear_actor);
}

void Zone::Leave(const Ptr<PlayerCharacter>& pc)
{
	size_t r = players_.erase(pc->GetEntityID());
	if (r == 0) return;

	pc->SetLocationZone(nullptr);

	// 주변에 퇴장 통보
	// TO DO : 공간 분할을 고려할것(격자, 쿼드트리 등)
	fb::FlatBufferBuilder fbb;
	std::vector<fb::Offset<fb::String>> entity_id_list;
	entity_id_list.push_back(fbb.CreateString(boost::uuids::to_string(pc->GetEntityID())));
	auto notify_disappear_actor = PWorld::CreateNotify_DisappearActorDirect(fbb, &entity_id_list);

	// 여러 명에게 보내기 때문에 공유버퍼 사용.
	auto buffer = PCS::MakeBuffer(fbb, notify_disappear_actor);
	// 지역의 플레이어에게 퇴장 메시지를 보낸다.
	for (auto& var : players_)
	{
		var.second->Send(buffer);
	}
}

void Zone::Update(float delta_time)
{
	for (auto& var : players_)
	{
		var.second->Update(delta_time);
	}
	for (auto& var : monsters_)
	{
		var.second->Update(delta_time);
	}
}
