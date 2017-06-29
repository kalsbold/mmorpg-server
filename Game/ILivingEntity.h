#pragma once
#include "Common.h"

using namespace boost;

class ILivingEntity
{
public:
    virtual bool IsDead() const = 0;
    virtual void Die() = 0;
    virtual int MaxHp() const = 0;
    virtual void MaxHp(int max_hp) = 0;
    virtual int Hp() const = 0;
    virtual void Hp(int hp) = 0;

    virtual signals2::connection ConnectDeathSignal(std::function<void(ILivingEntity*)> handler) = 0;
};