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

	// TO DO : ���� ������ ����Ұ�(����, ����Ʈ�� ��)
	// �ֺ� �÷��̾ ���� ����
	fb::FlatBufferBuilder fbb;
	auto remote_pc = pc->SerializeAs<PWorld::RemotePC>(fbb);
	std::vector<fb::Offset<PWorld::RemotePC>> remote_pc_list;
	remote_pc_list.push_back(remote_pc);
	auto notify_appear_actor = PWorld::CreateNotify_AppearActorDirect(fbb, &remote_pc_list);

	// ���� ���� ������ ������ �������� ���.
	auto buffer = PCS::MakeBuffer(fbb, notify_appear_actor);
	// ������ �÷��̾�� ���� �޽����� ������.
	for (auto& var : players_)
	{
		var.second->Send(buffer);
	}

	// �ڽſ��� �ֺ� ������Ʈ ������ ������.
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

	// �ֺ��� ���� �뺸
	// TO DO : ���� ������ ����Ұ�(����, ����Ʈ�� ��)
	fb::FlatBufferBuilder fbb;
	std::vector<fb::Offset<fb::String>> entity_id_list;
	entity_id_list.push_back(fbb.CreateString(boost::uuids::to_string(pc->GetEntityID())));
	auto notify_disappear_actor = PWorld::CreateNotify_DisappearActorDirect(fbb, &entity_id_list);

	// ���� ���� ������ ������ �������� ���.
	auto buffer = PCS::MakeBuffer(fbb, notify_disappear_actor);
	// ������ �÷��̾�� ���� �޽����� ������.
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
