#pragma once
#include "Common.h"
#include "Actor.h"
#include "RemoteWorldClient.h"
#include "protocol_cs_helper.h"

namespace PCS = ProtocolCS;

// 플레이어 캐릭터
class Hero : public Actor
{
public:
    Hero(const uuid& entity_id, RemoteWorldClient* rc, const db::Hero& db_data);
    virtual ~Hero();

    RemoteWorldClient* GetRemoteClient();

    void Send(const uint8_t * data, size_t size);
    template <typename BufferT>
    void Send(BufferT&& data)
    {
        rc_->Send(std::forward<BufferT>(data));
    }

    void SetToDB(db::Hero& db_data);

    void UpdateToDB(const Ptr<MySQLPool>& db);

    virtual void SetZone(Zone* zone) override;

    void Move(const Vector3& position, float rotation, const Vector3& velocity);

    void Skill()
    {

    }

    void OnDamage()
    {

    }

    fb::Offset<PCS::World::Actor> SerializeAsActor(fb::FlatBufferBuilder& fbb) const override;

    fb::Offset<PCS::World::Hero> SerializeAsHero(fb::FlatBufferBuilder& fbb) const;

    virtual void Update(double delta_time) override
    {

    }

    std::function<void(const Vector3&)> poistion_update_handler;

private:
    void InitAttribute(const db::Hero& db_data);

    RemoteWorldClient* rc_;

public:
    int            uid_;
    ClassType      class_type_;
    int            exp_;
    int            level_;
    int            max_hp_;
    int            hp_;
    int            max_mp_;
    int            mp_;
    int            att_;
    int            def_;
    int            map_id_;
};