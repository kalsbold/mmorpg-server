#pragma once
#include <gisunnet\gisunnet.h>
#include <flatbuffers\flatbuffers.h>
#include "game_message_generated.h"

namespace mmog {

using namespace gisunnet;
using namespace flatbuffers;
using namespace protocol;

template <typename T>
void Send(const Ptr<Session>& session, FlatBufferBuilder& fbb, Offset<T>& message)
{
	auto net_message = CreateNetMessage(fbb, MessageTTraits<T>::enum_value, message.Union());
	fbb.Finish(net_message);

	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

template <typename MessageT>
void Send(const Ptr<Session>& session, const MessageT& message)
{
	flatbuffers::FlatBufferBuilder fbb;
	auto offset = MessageT::TableType::Pack(fbb, &message);
	auto net_message = CreateNetMessage(fbb, MessageTTraits<MessageT::TableType>::enum_value, offset.Union());
	fbb.Finish(net_message);

	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

}