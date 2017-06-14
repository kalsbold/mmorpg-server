#pragma once
#include "Common.h"
#include "protocol_cs_generated.h"

using namespace gisun;

namespace ProtocolCS {

	template <typename T>
	void FinishMessageRoot(flatbuffers::FlatBufferBuilder& fbb, const T& message)
	{
		auto offset_message = T::TableType::Pack(fbb, &message);
		auto offset_root = CreateMessageRoot(fbb, MessageTypeTraits<T::TableType>::enum_value, offset_message.Union());
		FinishMessageRootBuffer(fbb, offset_root);
	}

	template <typename T>
	Ptr<net::Buffer> MakeBuffer(const T& message)
	{
		flatbuffers::FlatBufferBuilder fbb;
		FinishMessageRoot(fbb, message);

		auto buffer = std::make_shared<net::Buffer>(fbb.GetSize());
		buffer.WriteBytes(fbb.GetBufferPointer(), fbb.GetSize());

		return std::move(buffer);
	}

	template <typename Peer, typename T>
	void Send(Peer& peer, const T& message)
	{
		flatbuffers::FlatBufferBuilder fbb;
		FinishMessageRoot(fbb, message);
		peer.Send(fbb.GetBufferPointer(), fbb.GetSize());
	}

	//template <typename T1, typename T2>
	//T1 cast_as(const T2& rhs) {}

	//template <>
	//Vec3 cast_as<Vec3>(const Vector3& rhs)
	//{
	//	return Vec3(rhs.X, rhs.Y, rhs.Z);
	//}
}
