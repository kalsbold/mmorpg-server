#pragma once
#include <map>
#include <gisunnet/gisunnet.h>
#include <flatbuffers/flatbuffers.h>

using namespace gisunnet;

template<typename MessageType, typename Message>
class MessageHandlerRegistry
{
public:
	using MessageHandler = std::function<void(const Ptr<Session>&, const Message&)>;

	void Register(const MessageType& message_type, const MessageHandler& message_handler)
	{
		handler_map_.emplace(message_type, message_handler);
	}

	void Dispatch(const Ptr<Session>&, const MessageType& message_type, const Message& message)
	{
		auto it = handler_map_.find(message_type);
		if (it != handler_map.end())
		{

		}
	}

private:
	std::map<MessageType, MessageHandler> handler_map_;
};

