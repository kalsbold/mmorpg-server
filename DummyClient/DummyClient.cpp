// DummyClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "protocol_cs_generated.h"

using namespace ProtocolCS::World;

int main()
{
    HeroT hero;
    hero.entity_id = "1234";
    hero.name = "aaa";

    ActorTypeUnion actor;
    actor.Set(std::move(hero));

    flatbuffers::FlatBufferBuilder fbb;
    auto offset = CreateNotify_Appear(fbb, actor.type, actor.Pack(fbb));

    auto offset_root = ProtocolCS::CreateMessageRoot(fbb, ProtocolCS::MessageType::World_Notify_Appear, offset.Union());
    FinishMessageRootBuffer(fbb, offset_root);

    const ProtocolCS::MessageRoot* root = ProtocolCS::GetMessageRoot(fbb.GetBufferPointer());
    auto message = root->message_as_World_Notify_Appear();
    const ProtocolCS::World::Hero* h = message->entity_as_Hero();

    std::cout << h->entity_id()->c_str() << std::endl;
    std::cout << h->name()->c_str() << std::endl;

    return 0;
}

