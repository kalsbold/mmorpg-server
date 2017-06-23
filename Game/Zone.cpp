#include "stdafx.h"
#include "Zone.h"
#include "RemoteClient.h"
#include "Hero.h"
#include "Monster.h"
#include "World.h"
#include "StaticCachedData.h"
#include "protocol_cs_helper.h"


Zone::Zone(const uuid & entity_id, const Map & map_data, World * owner)
    : entity_id_(entity_id), map_data_(map_data), owner_(owner)
{}

Zone::~Zone()
{}

bool Zone::Enter(const Ptr<Hero>& pc)
{
    auto r = players_.emplace(pc->GetEntityID(), pc);
    if (!r.second) return false;

    pc->SetCurrentZone(this);

    // TO DO : 공간 분할을 고려할것(격자, 쿼드트리 등)
    // 주변 플레이어에 입장 통지
    fb::FlatBufferBuilder fbb;
    std::vector<fb::Offset<PWorld::Actor>> actor_list;

    auto hero_offset = pc->Serialize(fbb);
    actor_list.push_back(PWorld::CreateActor(fbb, PWorld::ActorType::Hero, hero_offset.Union()));
    auto notify_appear = PWorld::CreateNotify_AppearDirect(fbb, &actor_list);
    PCS::FinishMessageRoot(fbb, notify_appear);

    // 지역의 플레이어에게 보낸다.
    for (auto& var : players_)
    {
        var.second->Send(fbb.GetBufferPointer(), fbb.GetSize());
    }

    return true;
}

void Zone::Leave(const Ptr<Hero>& pc)
{
    size_t r = players_.erase(pc->GetEntityID());
    if (r == 0) return;

    pc->SetCurrentZone(nullptr);

    // 주변에 퇴장 통보
    // TO DO : 공간 분할을 고려할것(격자, 쿼드트리 등)
    fb::FlatBufferBuilder fbb;
    std::vector<fb::Offset<fb::String>> entity_id_list;
    entity_id_list.push_back(fbb.CreateString(boost::uuids::to_string(pc->GetEntityID())));
    auto notify_disappear = PWorld::CreateNotify_DisappearDirect(fbb, &entity_id_list);
    PCS::FinishMessageRoot(fbb, notify_disappear);

    // 지역의 플레이어에게 퇴장 메시지를 보낸다.
    for (auto& var : players_)
    {
        var.second->Send(fbb.GetBufferPointer(), fbb.GetSize());
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
