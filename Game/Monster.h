#pragma once
#include "Common.h"
#include "Actor.h"
#include "ILivingEntity.h"
#include "protocol_cs_helper.h"

using namespace boost;
namespace PCS = ProtocolCS;

class Monster : public Actor, public ILivingEntity
{
public:
	Monster(const uuid& entity_id);
	virtual ~Monster();

    void Init(const db::Monster& db_data);

    fb::Offset<PCS::World::Actor> SerializeAsActor(fb::FlatBufferBuilder& fbb) const override;

    fb::Offset<PCS::World::Monster> SerializeAsMonster(fb::FlatBufferBuilder& fbb) const;


    virtual void Update(double delta_time) override
    {

    }

    int Uid();
    int TypeId();
    int Level();
    int MaxMp();
    int Mp();
    int Att();
    int Def();

    // Inherited via ILivingEntity
    virtual bool IsDead() const override;
    virtual void Die() override;
    virtual int MaxHp() const override;
    virtual void MaxHp(int max_hp) override;
    virtual int Hp() const override;
    virtual void Hp(int hp) override;
    virtual signals2::connection ConnectDeathSignal(std::function<void(ILivingEntity*)> handler) override;

private:
    signals2::signal<void(ILivingEntity*)> death_signal_;

    int            uid_;
	int			   type_id_;
	int            level_;
	int            max_hp_;
	int            hp_;
	int            max_mp_;
	int            mp_;
	int            att_;
	int            def_;
	int            map_id_;
};
