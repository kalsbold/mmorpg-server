#pragma once
#include <memory>
#include <thread>
#include <mutex>
#include <boost/serialization/singleton.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <gisunnet/types.h>

namespace gisunnet
{

	enum CloseReason {
		ActiveClose = 0,	// 서버에서 연결을 끊음.
		Disconnected,		// 클라이언트에서 연결이 끊김.
		Timeout,			//
	};

	class Session;
	class Message;

	class Server
	{
	public:
		enum class State
		{
			Uninit = 0,
			Ready,
			Start,
			Stop,
		};

		// Handler to be notified on session creation.
		using SessionOpenedHandler = function<void(const Ptr<Session>&/*session*/)>;

		// Handler to be notified on session close. This one ignores the close reason.
		using SessionClosedHandler = function<void(const Ptr<Session>&/*session*/, CloseReason)>;

		// Message handler type.
		using MessageHandler = function<void(const Ptr<Session>&/*session*/, const Ptr<Message>&/*message*/)>;

		// Create server instance.
		static Ptr<Server> Create();

		Server(const Server&) = delete;
		Server& operator=(const Server&) = delete;

		virtual ~Server();

		virtual void Start(uint16_t port) = 0;
		virtual void Start(string address, uint16_t port) = 0;

		virtual void Stop() = 0;

		virtual State GetState() = 0;
		
		SessionOpenedHandler	sessionOpenedHandler;
		SessionClosedHandler	sessionClosedHandler;
		MessageHandler			messageHandler;

	protected:
		Server();
	};
}