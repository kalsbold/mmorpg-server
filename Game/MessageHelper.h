#pragma once
#include <gisunnet.h>
#include "protocol_generated.h"

namespace helper{

using namespace gisun;
namespace fb = flatbuffers;

template <typename T>
void Send(const Ptr<net::Session>& session, fb::FlatBufferBuilder& fbb, fb::Offset<T>& message)
{
	auto net_message = protocol::CreateNetMessage(fbb, protocol::MessageTTraits<T>::enum_value, message.Union());
	fbb.Finish(net_message);

	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

template <typename T>
void Send(const Ptr<net::Session>& session, const T& message)
{
	flatbuffers::FlatBufferBuilder fbb;
	auto offset = T::TableType::Pack(fbb, &message);
	auto net_message = CreateNetMessage(fbb, protocol::MessageTTraits<T::TableType>::enum_value, offset.Union());
	fbb.Finish(net_message);

	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

}