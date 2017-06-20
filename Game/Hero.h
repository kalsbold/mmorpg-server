#pragma once
#include "Common.h"
#include "Actor.h"
#include "RemoteClient.h"

// 플레이어 캐릭터
class Hero : public Actor
{
public:
    Hero(const uuid& entity_id, RemoteClient* rc, const db::Hero& db_data);
    virtual ~Hero();

    RemoteClient* GetRemoteClient()
    {
        return rc_;
    }

    void Send(const uint8_t * data, size_t size)
    {
        rc_->Send(data, size);
    }

    template <typename BufferT>
    void Send(BufferT&& data)
    {
        rc_->Send(std::forward<BufferT>(data));
    }

    virtual void SetLocationZone(Zone* zone) override;

    virtual void Update(double delta_time) override
    {

    }

    void SetTo(db::Hero& db_data)
    {
        db_data.uid = uid_;
        db_data.exp = exp_;
        db_data.level = level_;
        db_data.max_hp = max_hp_;
        db_data.hp = hp_;
        db_data.max_mp = max_mp_;
        db_data.mp = mp_;
        db_data.att = att_;
        db_data.def = def_;
        db_data.map_id = map_id_;
        db_data.pos = GetPosition();
        db_data.rotation = GetRotation();
    }

    void UpdateToDB(const Ptr<MySQLPool>& db)
    {
        db::Hero db_data;
        SetTo(db_data);
        db_data.Update(db);
    }

    fb::Offset<PWorld::Hero> Serialize(fb::FlatBufferBuilder& fbb) const
    {
        ProtocolCS::Vec3 pos(GetPosition().X, GetPosition().Y, GetPosition().Z);
        return PWorld::CreateHeroDirect(fbb,
            boost::uuids::to_string(GetEntityID()).c_str(),
            uid_,
            GetName().c_str(),
            (ProtocolCS::ClassType)class_type_,
            exp_,
            level_,
            max_hp_,
            hp_,
            max_mp_,
            mp_,
            att_,
            def_,
            map_id_,
            &pos,
            GetRotation()
        );
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

    RemoteClient* rc_;

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