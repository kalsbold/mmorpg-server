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
    void Send(BufferT&& data);

    void SetToDB(db::Hero& db_data);
    void UpdateToDB(const Ptr<MySQLPool>& db);

    fb::Offset<PWorld::Hero> Serialize(fb::FlatBufferBuilder& fbb) const;

    virtual void SetCurrentZone(Zone* zone) override;

    virtual void Update(double delta_time) override
    {

    }

    void Move(const Vector3& position, float rotation, const Vector3& velocity);

    void Skill()
    {

    }

    void OnDamage()
    {

    }

private:
    void InitAttribute(const db::Hero& db_data)
    {
        uid_ = db_data.uid;
        SetName(db_data.name);
        class_type_ = db_data.class_type;
        exp_ = db_data.exp;
        level_ = db_data.level;
        max_hp_ = db_data.max_hp;
        hp_ = db_data.hp;
        max_mp_ = db_data.max_mp;
        mp_ = db_data.mp;
        att_ = db_data.att;
        def_ = db_data.def;
        map_id_ = db_data.map_id;
        SetPosition(db_data.pos);
        SetRotation(db_data.rotation);
    }

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

template<typename BufferT>
inline void Hero::Send(BufferT && data)
{
    rc_->Send(std::forward<BufferT>(data));
}