#pragma once
#include "Common.h"
#include "Actor.h"
#include "ILivingEntity.h"
#include "RemoteWorldClient.h"
#include "protocol_cs_helper.h"

using namespace boost;
namespace PCS = ProtocolCS;

// 플레이어 캐릭터
class Hero : public Actor, public ILivingEntity
{
public:
    Hero(const uuid& entity_id, RemoteWorldClient* rc);
    virtual ~Hero();

    void Init(const db::Hero& db_data);

    RemoteWorldClient* GetRemoteClient();

    void Send(const uint8_t * data, size_t size);
    template <typename BufferT>
    void Send(BufferT&& data)
    {
        rc_->Send(std::forward<BufferT>(data));
    }

    void SetToDB(db::Hero& db_data);
    void UpdateToDB(const Ptr<MySQLPool>& db);

    void ActionMove(const Vector3& position, float rotation, const Vector3& velocity);

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

    int Uid() { return uid_; }
    ClassType Type() { return class_type_; }
    int Exp() { return exp_; }
    int Level() { return level_; }
    int MaxMp() { return max_mp_; }
    int Mp() { return mp_; }
    int Att() { return att_; }
    int Def() { return def_; }

    // Inherited via Actor
    virtual void SetZone(Zone* zone) override;

    // Inherited via ILivingEntity
    virtual bool IsDead() const override;
    virtual void Die() override;
    virtual int MaxHp() const override;
    virtual void MaxHp(int max_hp) override;
    virtual int Hp() const override;
    virtual void Hp(int hp) override;
    virtual signals2::connection ConnectDeathSignal(std::function<void(ILivingEntity*)> handler) override;

private:
    RemoteWorldClient* rc_;

    signals2::signal<void(ILivingEntity*)> death_signal_;

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