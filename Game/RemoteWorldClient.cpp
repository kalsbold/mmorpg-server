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
    // �ɸ��� ���� DB Update.
    if (hero_)
        hero_->UpdateToDB(GetDB());
}

// ���� ó��. ���� DB Update �� �� �Ѵ�.
inline void RemoteWorldClient::Dispose()
{
    bool exp = false;
    if (!disposed_.compare_exchange_strong(exp, true))
        return;

    if (hero_)
    {
        // ���忡�� ������.
        Zone* zone = hero_->GetLocationZone();
        if (zone != nullptr)
        {
            zone->GetWorld()->Dispatch([zone, hero = hero_] {
                zone->Leave(hero);
            });
        }
    }
    // DB ������Ʈ
    UpdateToDB();
}


// �޽��� ó��
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

        // �ִ� �ӵ����� ������ �����ϼ� ����. 
        //if (distance(hero_->GetPosition(), position) > HERO_MOVE_SPEED * delta_time)
        //    return;
        auto& mapData = zone->MapData();
        if (mapData.height > pos.Z && 0 < pos.Z && mapData.width > pos.X && 0 < pos.X)
        {
            hero_->SetPosition(pos);
        }
        hero_->SetRotation(rotation);

        pos = hero_->GetPosition();

        // �ֺ� �÷��̾ ����
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

        // ������ �÷��̾�� ������.
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
