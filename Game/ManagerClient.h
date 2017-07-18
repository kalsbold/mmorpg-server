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

// 매니저 서버에 접속하는 클라이언트.
// 인증 요청, 확인 및 응답을 받는다.
class ManagerClient
{
public:
	ManagerClient(const net::ClientConfig& config, IServer* owner, ServerType type, std::string client_name);
	virtual ~ManagerClient();

    // 클라이언트 이름.
	std::string GetName() { return name_; }
	// 접속.
	void Connect(const std::string& address, uint16_t port);
	// 종료.
	void Disconnect();
	// 종료될 때 까지 대기.
	void Wait();
    // 이 크라이언트가 실행되고 있는 IoServiceLoop 객체.
	const Ptr<net::IoServiceLoop>& GetIoServiceLoop() { return ios_loop_; }

    // 인증키 생성을 요청.
	void RequestGenerateCredential(int session_id, int account_uid);
    // 인증키 검증을 요청.
	void RequestVerifyCredential(int session_id, const uuid& credential);
    // 유저의 로그아웃을 통지.
	void NotifyUserLogout(int account_uid);

	std::function<void(PSS::ErrorCode error)> OnLoginManagerServer;
	std::function<void()> OnDisconnectManagerServer;
	std::function<void(int session_id, const uuid& credential)> OnReplyGenerateCredential;
	std::function<void(PSS::ErrorCode error, int session_id, const uuid& credential, int account_uid)> OnReplyVerifyCredential;

private:
    // Network message handler type.
    using MessageHandler = std::function<void(const PSS::MessageRoot* message_root)>;

    // 프레임 업데이트
    void DoUpdate(double delta_time) {};
    void ScheduleNextUpdate(const time_point& now, const duration& timestep);

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

    // Handlers
    void RegisterHandlers();

    void HandleConnected(bool success);
    void HandleDisconnected();
	void HandleMessage(const uint8_t* buf, size_t bytes);

	std::mutex mutex_;

	Ptr<net::IoServiceLoop> ios_loop_;
	Ptr<net::NetClient> net_client_;
	Ptr<timer_type> update_timer_;

	IServer*	owner_;
	ServerType	type_;
	std::string name_;

	std::map<PSS::MessageType, MessageHandler> message_handlers_;

	// TO DO : Manager 서버에 접속해 있는 다른 Manager 클라이언트들 정보.
};
