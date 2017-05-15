#pragma once

#include <future>
#include "gisunnet/types.h"
#include "gisunnet/network/Session.h"
#include "gisunnet/network/Configuration.h"

namespace gisunnet {

class NetServer
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
	using SessionClosedHandler = function<void(const Ptr<Session>&/*session*/, const CloseReason&)>;

	// Receive handler type.
	using MessageHandler = function<void(const Ptr<Session>&/*session*/, const uint8_t*, size_t)>;

	// Create server instance.
	static Ptr<NetServer> Create(const Configuration& config);

	NetServer(const NetServer&) = delete;
	NetServer& operator=(const NetServer&) = delete;

	virtual ~NetServer() {};

	virtual void Start(uint16_t port) = 0;
	virtual void Start(string address, uint16_t port) = 0;
	virtual void Stop() = 0;

	virtual State GetState() = 0;
	virtual Ptr<IoServiceLoop> GetIoServiceLoop() = 0;

	virtual Ptr<Session> Find(const uuid& id) = 0;

	virtual void RegisterSessionOpenedHandler(const SessionOpenedHandler& handler) = 0;
	virtual void RegisterSessionClosedHandler(const SessionClosedHandler& handler) = 0;
	virtual void RegisterMessageHandler(const MessageHandler& handler) = 0;

protected:
	NetServer() {};
};

} // namespace gisunnet