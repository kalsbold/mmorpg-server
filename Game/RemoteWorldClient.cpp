#include "stdafx.h"
#include "RemoteWorldClient.h"
#include "WorldServer.h"
#include "World.h"
#include "Hero.h"
#include "Monster.h"
#include "CachedResources.h"
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

    auto db_hero = GetDBHero();
    if (!db_hero)
    {
        PCS::World::Notify_EnterFailedT reply;
        reply.error_code = PCS::ErrorCode::WORLD_CANNOT_LOAD_HERO;
        PCS::Send(*this, reply);
        return;
    }
    // 캐릭터를 생성하고 처음 입장하는 경우
    if (db_hero->map_id == 0)
    {
        // 시작 지점을 찾는다.
        auto spawn_table = HeroSpawnTable::GetInstance().GetAll();
        auto iter = std::find_if(spawn_table.begin(), spawn_table.end(), [](auto& value) {
            return (value.second.map_id == 1001);
        });
        if (iter == spawn_table.end())
        {
            PCS::World::Notify_EnterFailedT reply;
            reply.error_code = PCS::ErrorCode::WORLD_CANNOT_ENTER_ZONE;
            PCS::Send(*this, reply);
            return;
        }
        // 맵, 좌표 셋
        db_hero->map_id = iter->second.map_id;
        db_hero->pos = iter->second.pos;
    }

    Zone* zone = GetWorld()->FindFieldZone(db_hero->map_id);
    if (zone == nullptr)
    {
        PCS::World::Notify_EnterFailedT reply;
        reply.error_code = PCS::ErrorCode::WORLD_CANNOT_FIND_ZONE;
        PCS::Send(*this, reply);
        return;
    }

    // 케릭터 인스턴스 생성
    auto hero = std::make_shared<Hero>(random_generator()(), this);
    hero->Init(*db_hero);
    SetHero(hero);

    hero->ConnectDeathSignal(std::bind(&RemoteWorldClient::OnHeroDeath, this));

    // 월드 입장
    SetState(RemoteWorldClient::State::WorldEntering);

    BOOST_LOG_TRIVIAL(info) << "World Entring . account_uid : " << GetAccount()->uid << " hero_name: " << hero->GetName();

    GetWorld()->Dispatch([this, zone, hero] {

        zone->Enter(hero, hero->GetPosition());
        this->SetState(RemoteWorldClient::State::WorldEntered);

        // 입장 성공 메시지를 보낸다
        fb::FlatBufferBuilder fbb;
        auto offset_msg = PCS::World::CreateNotify_EnterSuccess(fbb, hero->SerializeAsHero(fbb));
        PCS::Send(*this, fbb, offset_msg);

        // 관심 지역을 만든다
        this->interest_area_ = std::make_shared<ClientInterestArea>(this, zone);
        this->interest_area_->ViewDistance(Vector3(20.0f, 1.0f, 20.0f));
        hero->poistion_update_signal.connect(std::bind(&RemoteWorldClient::OnUpdateHeroPosition, this, std::placeholders::_1));
        hero->poistion_update_signal(hero->GetPosition());
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
        hero->ActionMove(position, rotation, velocity);
    });
}

void RemoteWorldClient::ActionSkill(const PCS::World::Request_ActionSkill * message)
{
    auto delta_time_duration = clock_type::now() - last_position_update_time_;
    if (delta_time_duration < 50ms) { return; }

    if (!hero_) return;

    last_position_update_time_ = clock_type::now();

    auto skill_id = message->skill();
    auto rotation = message->rotation();
    auto fb_targets = message->targets();
    std::vector<uuid> targets;
    targets.reserve(fb_targets->Length());
    for (const auto& e : (*fb_targets))
    {
        targets.emplace_back(boost::uuids::string_generator()(e->c_str()));
    }

    GetWorld()->Dispatch([this, hero = hero_, skill_id, rotation, targets = std::move(targets)]() {
        hero->ActionSkill(skill_id, rotation, targets);
    });
}

void RemoteWorldClient::Respawn()
{
    // 조건 검사
    if (!(hero_ && hero_->IsDead() && respawn_timer_))
        return;

    respawn_timer_->cancel();
    respawn_timer_.reset();

    GetWorld()->Dispatch([this, hero = hero_]() {
        if (!(hero_->IsDead()))
            return;

        // 시작 지점을 찾는다.
        auto spawn_table = HeroSpawnTable::GetInstance().GetAll();
        auto iter = std::find_if(spawn_table.begin(), spawn_table.end(), [&](auto& value) {
            return (value.second.map_id == hero->MapId());
        });
        if (iter == spawn_table.end())
        {
            return;
        }
        db::HeroSpawn& spawn_info = iter->second;

        hero->Hp(hero->MaxHp()); // HP 회복
        hero->Spawn(spawn_info.pos);

        BOOST_LOG_TRIVIAL(info) << "Spawn Hero : " << hero->GetName();

        // 주변에 통지
        PCS::World::ActorT actor_data;
        hero->SerializeAsActorT(actor_data);
        PCS::World::Notify_UpdateT update_msg;
        update_msg.entity_id = uuids::to_string(hero->GetEntityID());
        update_msg.update_data.Set(std::move(actor_data));
        hero->PublishActorUpdate(&update_msg);
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
            hero->poistion_update_signal.disconnect_all_slots();
        });
    }
}

void RemoteWorldClient::OnHeroDeath()
{
    if (!hero_)
        return;

    BOOST_LOG_TRIVIAL(info) << "On DeathSignal. " << hero_->GetName();

    // 10초후 리스폰
    respawn_timer_ = GetWorld()->RunAfter(10s, [this, hero = hero_](auto& timer) {
        if (!(hero_->IsDead()))
            return;

        // 시작 지점을 찾는다.
        auto spawn_table = HeroSpawnTable::GetInstance().GetAll();
        auto iter = std::find_if(spawn_table.begin(), spawn_table.end(), [&](auto& value) {
            return (value.second.map_id == hero->MapId());
        });
        if (iter == spawn_table.end())
        {
            return;
        }
        db::HeroSpawn& spawn_info = iter->second;

        hero->Hp(hero->MaxHp()); // HP 회복
        hero->Spawn(spawn_info.pos);
        
        BOOST_LOG_TRIVIAL(info) << "Spawn Hero : " << hero->GetName();

        // 주변에 통지
        PCS::World::ActorT actor_t;
        hero->SerializeAsActorT(actor_t);
        PCS::World::Notify_UpdateT update_msg;
        update_msg.entity_id = uuids::to_string(hero->GetEntityID());
        update_msg.update_data.Set(std::move(actor_t));
        hero->PublishActorUpdate(&update_msg);
    });
}

const Ptr<MySQLPool>& RemoteWorldClient::GetDB()
{
    return owner_->GetDB();
}
