#pragma once
#include <gisunnet\gisunnet.h>
#include <flatbuffers\flatbuffers.h>
#include "game_message_generated.h"

namespace mmog {

using namespace gisunnet;
using namespace flatbuffers;
using namespace protocol;

template <typename Message>
void Send(const Ptr<Session>& session, FlatBufferBuilder& fbb, Offset<Message>& message)
{
	auto net_message = CreateNetMessage(fbb, MessageTTraits<Message>::enum_value, message.Union());
	fbb.Finish(net_message);

	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

template <typename MessageType>
void Send(const Ptr<Session>& session, const MessageType& message)
{
	flatbuffers::FlatBufferBuilder fbb;
	auto offset = MessageType::TableType::Pack(fbb, &message);
	auto net_message = CreateNetMessage(fbb, MessageTTraits<MessageType::TableType>::enum_value, offset.Union());
	fbb.Finish(net_message);

	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

}