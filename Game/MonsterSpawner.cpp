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
    // 몬스터 인스턴스 생성하고 스폰.
    for (auto& e : spawn_table)
    {
        if (e.second.map_id == zone_->MapID())
        {
            //spawn_talbe_.emplace(e.first, &e.second);
            Spawn(e.second.uid);
            
            // TEST CODE
            // break;
        }
    }
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
    // 몬스터 인스턴스 생성
    auto new_monster = std::make_shared<Monster>(boost::uuids::random_generator()());
    new_monster->Init(*db_monster);

    // 랜덤 위치
    std::uniform_real_distribution<float> dist {-3.0f, 3.0f};
    std::random_device rd;
    std::default_random_engine rng {rd()};
    Vector3 position((db_spawn->pos).X + dist(rng), 0.0f, (db_spawn->pos).Z + dist(rng));

    // Zone 입장
    zone_->Enter(new_monster, position);
    new_monster->InitAI();
    BOOST_LOG_TRIVIAL(info) << "Spawn Monster. spawn_uid: " << spawn_uid << " entity_id:" << new_monster->GetEntityID();

    spawn_monsters_[spawn_uid] = new_monster;
    new_monster->ConnectDeathSignal([this, db_spawn, spawn_uid](ILivingEntity* entity)
    {
        auto monster = spawn_monsters_[spawn_uid];
        BOOST_LOG_TRIVIAL(info) << "On DeathSignal. spawn_uid: " << spawn_uid << " entity_id:" << monster->GetEntityID();
            
        if (monster)
        {
            spawn_monsters_[spawn_uid] = nullptr;
            // 5초후 퇴장
            world_->RunAfter(5s, [this, monster](auto timer) {
                zone_->Exit(monster->GetEntityID());
            });
            // 리스폰
            world_->RunAfter(db_spawn->interval_s, [this, spawn_uid](auto timer) {
                Spawn(spawn_uid);
            });
        }
    });
}
