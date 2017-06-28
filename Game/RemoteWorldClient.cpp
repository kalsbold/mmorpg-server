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
    // 케릭터 상태 DB Update.
    if (hero_)
        hero_->UpdateToDB(GetDB());
}

// 종료 처리. 상태 DB Update 등 을 한다.
void RemoteWorldClient::Dispose()
{
    bool exp = false;
    if (!disposed_.compare_exchange_strong(exp, true))
        return;

    ExitWorld();
    // DB 업데이트
    UpdateToDB();
}


void RemoteWorldClient::EnterWorld()
{
    // 입장전 상태가 아님
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

    // 월드 입장
    SetState(RemoteWorldClient::State::WorldEntering);

    BOOST_LOG_TRIVIAL(info) << "World Entring . account_uid : " << GetAccount()->uid << " hero_name: " << hero->GetName();

    GetWorld()->Dispatch([this, zone, hero] {
        if (zone->Enter(hero, hero->GetPosition()))
        {
            this->SetState(RemoteWorldClient::State::WorldEntered);

            // 관심 지역을 만든다
            this->interest_area_ = std::make_shared<ClientInterestArea>(this, zone);
            this->interest_area_->ViewDistance(Vector3(20.0f, 1.0f, 20.0f));
            hero->poistion_update_handler = std::bind(&RemoteWorldClient::OnUpdateHeroPosition, this, std::placeholders::_1);
            hero->poistion_update_handler(hero->GetPosition());

            PCS::World::Notify_EnterSuccessT reply;
            PCS::Send(*this, reply);
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

// 메시지 처리
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

void RemoteWorldClient::ExitWorld()
{
    if (hero_)
    {
        // 월드에서 나간다.
        GetWorld()->Dispatch([this, hero = hero_] {
            Zone* zone = hero->GetZone();
            if (zone)
            {
                zone->Exit(hero);
            }
            // 관심지역 해제
            this->interest_area_ = nullptr;
            hero->poistion_update_handler = nullptr;
        });
    }
}

const Ptr<MySQLPool>& RemoteWorldClient::GetDB()
{
    return owner_->GetDB();
}
