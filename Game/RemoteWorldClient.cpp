#include "stdafx.h"
#include "RemoteWorldClient.h"
#include "WorldServer.h"
#include "World.h"
#include "Hero.h"
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

inline void RemoteWorldClient::UpdateToDB()
{
    // 케릭터 상태 DB Update.
    if (hero_)
        hero_->UpdateToDB(GetDB());
}

// 종료 처리. 상태 DB Update 등 을 한다.
inline void RemoteWorldClient::Dispose()
{
    bool exp = false;
    if (!disposed_.compare_exchange_strong(exp, true))
        return;

    if (hero_)
    {
        // 월드에서 나간다.
        Zone* zone = hero_->GetLocationZone();
        if (zone != nullptr)
        {
            zone->GetWorld()->Dispatch([zone, hero = hero_] {
                zone->Leave(hero);
            });
        }
    }
    // DB 업데이트
    UpdateToDB();
}


// 메시지 처리
void RemoteWorldClient::OnActionMove(const Vector3 & position, float rotation, const Vector3 & velocity)
{
    auto delta_time_duration = clock_type::now() - last_position_update_time_;
    if (delta_time_duration < 50ms)
    {
        return;
    }

    if (!hero_) return;

    Zone* zone = hero_->GetLocationZone();
    if (zone == nullptr) return;

    last_position_update_time_ = clock_type::now();

    zone->GetWorld()->Dispatch([this, zone, position, rotation, velocity]() {
        Vector3 pos(position);
        pos.Y = 0;

        // 최대 속도보다 빠르게 움직일수 없다. 
        //if (distance(hero_->GetPosition(), position) > HERO_MOVE_SPEED * delta_time)
        //    return;
        auto& mapData = zone->MapData();
        if (mapData.height > pos.Z && 0 < pos.Z && mapData.width > pos.X && 0 < pos.X)
        {
            hero_->SetPosition(pos);
        }
        hero_->SetRotation(rotation);

        pos = hero_->GetPosition();

        // 주변 플레이어에 통지
        fb::FlatBufferBuilder fbb;
        std::vector<fb::Offset<PWorld::MoveInfo>> list;

        auto offset = PWorld::CreateMoveInfoDirect(fbb,
            boost::uuids::to_string(hero_->GetEntityID()).c_str(),
            &PCS::Vec3(pos.X, pos.Y, pos.Z),
            hero_->GetRotation(),
            &PCS::Vec3(velocity.X, velocity.Y, velocity.Z));
        list.push_back(offset);

        auto notify = PWorld::CreateNotify_ActionMoveDirect(fbb, &list);
        PCS::FinishMessageRoot(fbb, notify);

        // 지역의 플레이어에게 보낸다.
        for (auto& var : zone->players_)
        {
            var.second->Send(fbb.GetBufferPointer(), fbb.GetSize());
        }
    });
}

inline const Ptr<MySQLPool>& RemoteWorldClient::GetDB()
{
    return owner_->GetDB();
}
