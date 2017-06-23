#include "stdafx.h"
#include "RemoteWorldClient.h"
#include "WorldServer.h"
#include "World.h"
#include "Hero.h"
#include "Monster.h"
#include "protocol_cs_helper.h"

RemoteWorldClient::RemoteWorldClient(const Ptr<net::Session>& net_session, WorldServer * owner)
    : RemoteClient(net_session)
    , owner_(owner)
    , state_(State::Connected)
    , disposed_(false)
{
    assert(owner != nullptr);
}

RemoteWorldClient::~RemoteWorldClient()
{
    Dispose();
}

World * RemoteWorldClient::GetWorld() { return owner_->GetWorld(); }

void RemoteWorldClient::UpdateToDB()
{
    // �ɸ��� ���� DB Update.
    if (hero_)
        hero_->UpdateToDB(GetDB());
}

// ���� ó��. ���� DB Update �� �� �Ѵ�.
void RemoteWorldClient::Dispose()
{
    bool exp = false;
    if (!disposed_.compare_exchange_strong(exp, true))
        return;

    if (hero_)
    {
        // ���忡�� ������.
        GetWorld()->Dispatch([hero = hero_] {
            Zone* zone = hero->GetCurrentZone();
            if (zone == nullptr) return;

            zone->Leave(hero);
        });
    }
    // DB ������Ʈ
    UpdateToDB();
}


void RemoteWorldClient::EnterWorld()
{
    // ������ ���°� �ƴ�
    if (GetState() != RemoteWorldClient::State::Connected)
    {
        PCS::World::Notify_EnterFailedT reply_msg;
        reply_msg.error_code = PCS::ErrorCode::WORLD_LOGIN_INVALID_STATE;
        PCS::Send(*this, reply_msg);
        return;
    }

    auto db_data = GetDBHero();
    auto hero = GetHero();
    if (!db_data || !hero)
    {
        PCS::World::Notify_EnterFailedT reply;
        reply.error_code = PCS::ErrorCode::WORLD_CANNOT_LOAD_HERO;
        PCS::Send(*this, reply);
        return;
    }

    Zone* zone = GetWorld()->FindFieldZone(db_data->map_id);
    if (zone == nullptr)
    {
        PCS::World::Notify_EnterFailedT reply;
        reply.error_code = PCS::ErrorCode::WORLD_CANNOT_FIND_ZONE;
        PCS::Send(*this, reply);
        return;
    }

    // ���� ����
    SetState(RemoteWorldClient::State::WorldEntering);

    BOOST_LOG_TRIVIAL(info) << "World Entring . account_uid : " << GetAccount()->uid << " hero_name: " << hero->GetName();

    GetWorld()->Dispatch([this, zone, hero] {
        if (zone->Enter(hero))
        {
            this->SetState(RemoteWorldClient::State::WorldEntered);

            PCS::World::Notify_EnterSuccessT reply;
            PCS::Send(*this, reply);

            this->NotifyAppearActorsToMe();
        }
        else
        {
            this->SetState(RemoteWorldClient::State::Connected);

            PCS::World::Notify_EnterFailedT reply;
            reply.error_code = PCS::ErrorCode::WORLD_CANNOT_ENTER_ZONE;
            PCS::Send(*this, reply);
        }
    });
}

// �޽��� ó��
void RemoteWorldClient::ActionMove(const PCS::World::Request_ActionMove * message)
{
    assert(message);

    auto delta_time_duration = clock_type::now() - last_position_update_time_;
    if (delta_time_duration < 50ms) { return; }

    if (!hero_) return;

    last_position_update_time_ = clock_type::now();

    auto* pos = message->position();
    auto* vel = message->velocity();
    auto position = Vector3(pos->x(), pos->y(), pos->z());
    auto rotation = message->rotation();
    auto velocity = Vector3(pos->x(), pos->y(), pos->z());

    GetWorld()->Dispatch([this, hero = hero_, position, rotation, velocity]() {
        hero->Move(position, rotation, velocity);
    });
}

// �ڽſ��� �ֺ� ������ �˸���.
void RemoteWorldClient::NotifyAppearActorsToMe()
{
    if (GetState() != RemoteWorldClient::State::WorldEntered)
        return;

    auto hero = GetHero();
    if (!hero) return;

    GetWorld()->Dispatch([this, hero = hero]() {
        Zone* zone = hero->GetCurrentZone();
        if (!zone) return;

        // �ڽſ��� �ֺ� ������Ʈ ������ ������.
        fb::FlatBufferBuilder fbb;
        std::vector<fb::Offset<PWorld::Actor>> actor_list;

        for (auto& var : zone->players_)
        {
            auto offset = var.second->Serialize(fbb);
            actor_list.push_back(PWorld::CreateActor(fbb, PWorld::ActorType::Hero, offset.Union()));
        }
        for (auto& var : zone->monsters_)
        {
            auto offset = var.second->Serialize(fbb);
            actor_list.push_back(PWorld::CreateActor(fbb, PWorld::ActorType::Monster, offset.Union()));
        }
        // �ڽſ��� ������.
        auto notify_appear = PWorld::CreateNotify_AppearDirect(fbb, &actor_list);
        PCS::Send(*(hero->GetRemoteClient()), fbb, notify_appear);
    });
}

const Ptr<MySQLPool>& RemoteWorldClient::GetDB()
{
    return owner_->GetDB();
}
