#include "stdafx.h"
#include "MonsterSpawner.h"
#include "World.h"
#include "Monster.h"
#include "CachedResources.h"

MonsterSpawner::MonsterSpawner(Zone * zone)
    : world_(zone->GetWorld()), zone_(zone)
{
}

MonsterSpawner::~MonsterSpawner()
{
}

void MonsterSpawner::Start()
{
    const auto spawn_table = MonsterSpawnTable::GetInstance().GetAll();

    for (auto& e : spawn_table)
    {
        if (e.second.map_id == zone_->MapID())
        {
            //spawn_talbe_.emplace(e.first, &e.second);
            Spawn(e.second.uid);
        }
    }

    // test code
    //world_->RunAfter(80s, [this](auto& timer) {
    //    for (auto& e : spawn_monsters_)
    //    {
    //        auto mon = e.second;
    //        if (mon)
    //        {
    //            mon->Die();
    //        }
    //    }
    //});
}

void MonsterSpawner::Spawn(int spawn_uid)
{
    if (spawn_monsters_[spawn_uid])
        return;

    const db::MonsterSpawn* db_spawn = MonsterSpawnTable::GetInstance().Get(spawn_uid);
    if (!db_spawn)
    {
        BOOST_LOG_TRIVIAL(info) << "Can not find MonsterSpawnTable : " << spawn_uid;
        return;
    }

    const db::Monster* db_monster = MonsterTable::GetInstance().Get(db_spawn->monster_uid);
    if (!db_monster)
    {
        BOOST_LOG_TRIVIAL(info) << "Can not find MonsterTable : " << db_spawn->monster_uid;
        return;
    }

    auto new_monster = std::make_shared<Monster>(boost::uuids::random_generator()());
    new_monster->Init(*db_monster);

    if (zone_->Enter(new_monster, db_spawn->pos))
    {
        BOOST_LOG_TRIVIAL(info) << "Spawn Monster. spawn_uid: " << spawn_uid << " entity_id:" << new_monster->GetEntityID();

        spawn_monsters_[spawn_uid] = new_monster;
        new_monster->ConnectDeathSignal([this, db_spawn, spawn_uid](ILivingEntity* entity)
        {
            auto monster = spawn_monsters_[spawn_uid];
            BOOST_LOG_TRIVIAL(info) << "On DeathSignal. spawn_uid: " << spawn_uid << " entity_id:" << monster->GetEntityID();
            
            if (monster)
            {
                spawn_monsters_[spawn_uid] = nullptr;

                world_->RunAfter(5s, [this, monster](auto timer) {
                    zone_->Exit(monster->GetEntityID());
                });

                world_->RunAfter(db_spawn->interval_s, [this, spawn_uid](auto timer) {
                    Spawn(spawn_uid);
                });
            }
        });
    }
}
