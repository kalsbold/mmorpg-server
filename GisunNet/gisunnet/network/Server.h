#pragma once
#include <memory>
#include <thread>
#include <mutex>
#include <boost/serialization/singleton.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include <gisunnet/types.h>
#include <gisunnet/network/Session.h>

namespace gisunnet
{

	

	class Message;

	class Server
	{
	public:
		enum class State
		{
			Ready = 0,
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
		
		virtual Ptr<Session> Find(const SessionID& id) = 0;

		virtual void RegisterSessionOpenedHandler(const SessionOpenedHandler& handler) = 0;
		virtual void RegisterSessionClosedHandler(const SessionClosedHandler& handler) = 0;
		virtual void RegisterMessageHandler(uint16_t message_type, const MessageHandler& handler) = 0;

	protected:
		Server();
	};
}