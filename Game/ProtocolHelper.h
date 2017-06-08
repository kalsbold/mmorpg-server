#pragma once
#include "Common.h"
#include "protocol_generated.h"

using namespace gisun;
namespace fb = flatbuffers;

template <typename T>
fb::Offset<protocol::NetMessage> CreateNetMessageHelper(fb::FlatBufferBuilder& fbb, fb::Offset<T>& message)
{
	return protocol::CreateNetMessage(fbb, protocol::MessageTTraits<T>::enum_value, message.Union());
}

void SendFlatBuffer(const Ptr<net::Session>& session, fb::FlatBufferBuilder& fbb)
{
	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

template <typename T>
void SendProtocolMessage(const Ptr<net::Session>& session, const T& message)
{
	flatbuffers::FlatBufferBuilder fbb;
	auto offset = T::TableType::Pack(fbb, &message);
	auto net_message = CreateNetMessage(fbb, protocol::MessageTTraits<T::TableType>::enum_value, offset.Union());
	fbb.Finish(net_message);

	SendFlatBuffer(session, fbb);
}

Ptr<net::Buffer> Buffer(fb::FlatBufferBuilder& fbb)
{
	auto buffer = std::make_shared<net::Buffer>(fbb.GetSize());
	buffer->WriteBytes(fbb.GetBufferPointer(), fbb.GetSize());
	return buffer;
}

template <typename T1, typename T2>
T1 cast_as(const T2& rhs) {}

template <>
protocol::Vec3 cast_as<protocol::Vec3>(const Vector3& rhs)
{
	return protocol::Vec3(rhs.X, rhs.Y, rhs.Z);
}