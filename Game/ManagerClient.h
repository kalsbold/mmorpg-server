#pragma once
#include <map>
#include <mutex>
#include <type_traits>
#include <chrono>
#include "Common.h"
#include "protocol_ss_generated.h"

namespace PSS = ProtocolSS;

enum ServerType : int
{
	None,
	Login_Server,
	World_Server
};

class IServer;

class ManagerClient
{
public:
	/*
	// The clock type.
	using clock = std::chrono::high_resolution_clock;
	// The duration type of the clock.
	using duration = clock::duration;
	// The time point type of the clock.
	using time_point = clock::time_point;
	// Timer type
	using timer = boost::asio::high_resolution_timer;
	*/

	ManagerClient(const net::ClientConfig& config, IServer* owner, ServerType type, std::string client_name);
	virtual ~ManagerClient();

	std::string GetName() { return name_; }

	// 접속 시작
	void Connect(const std::string& address, uint16_t port);
	// 종료
	void Disconnect();
	// 종료될 때 까지 대기
	void Wait();

	const Ptr<net::IoServiceLoop>& GetIoServiceLoop() { return ios_loop_; }

	void RequestGenerateCredential(int session_id, int account_uid);
	void RequestVerifyCredential(int session_id, const uuid& credential);
	void NotifyUserLogout(int account_uid);

	std::function<void(PSS::ErrorCode error)> OnLoginManagerServer;
	std::function<void()> OnDisconnectManagerServer;
	std::function<void(int session_id, const uuid& credential)> OnReplyGenerateCredential;
	std::function<void(PSS::ErrorCode error, int session_id, const uuid& credential, int account_uid)> OnReplyVerifyCredential;

private:
	void HandleConnected(bool success);
	void HandleDisconnected();

	template <typename T, typename Handler>
	void RegisterMessageHandler(Handler&& handler)
	{
		auto key = PSS::MessageTypeTraits<T>::enum_value;
		auto func = [handler = std::forward<Handler>(handler)](const PSS::MessageRoot* message_root)
		{
			auto* message = message_root->message_as<T>();
			handler(message);
		};

		message_handlers_.insert(std::pair<decltype(key), decltype(func)>(key, func));
	}

	void HandleMessage(const uint8_t* buf, size_t bytes);
	void RegisterHandlers();

	void ScheduleNextUpdate(const time_point& now, const duration& timestep);
	// 프레임 업데이트
	void DoUpdate(double delta_time) {};

	std::mutex mutex_;

	Ptr<net::IoServiceLoop> ios_loop_;
	Ptr<net::NetClient> net_client_;
	Ptr<timer> update_timer_;

	IServer*	owner_;
	ServerType	type_;
	std::string name_;

	// Network message handler type.
	using MessageHandler = std::function<void(const PSS::MessageRoot* message_root)>;
	std::map<PSS::MessageType, MessageHandler> message_handlers_;

	// TO DO : Manager 서버에 접속해 있는 다른 Manager 클라이언트들 정보.
};
