#pragma once
#include <GisunNet.h>
#include <protocol_ss_generated.h>

using namespace gisun;

namespace ProtocolSS {

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

}
