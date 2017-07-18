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

// �Ŵ��� ������ �����ϴ� Ŭ���̾�Ʈ.
// ���� ��û, Ȯ�� �� ������ �޴´�.
class ManagerClient
{
public:
	ManagerClient(const net::ClientConfig& config, IServer* owner, ServerType type, std::string client_name);
	virtual ~ManagerClient();

    // Ŭ���̾�Ʈ �̸�.
	std::string GetName() { return name_; }
	// ����.
	void Connect(const std::string& address, uint16_t port);
	// ����.
	void Disconnect();
	// ����� �� ���� ���.
	void Wait();
    // �� ũ���̾�Ʈ�� ����ǰ� �ִ� IoServiceLoop ��ü.
	const Ptr<net::IoServiceLoop>& GetIoServiceLoop() { return ios_loop_; }

    // ����Ű ������ ��û.
	void RequestGenerateCredential(int session_id, int account_uid);
    // ����Ű ������ ��û.
	void RequestVerifyCredential(int session_id, const uuid& credential);
    // ������ �α׾ƿ��� ����.
	void NotifyUserLogout(int account_uid);

	std::function<void(PSS::ErrorCode error)> OnLoginManagerServer;
	std::function<void()> OnDisconnectManagerServer;
	std::function<void(int session_id, const uuid& credential)> OnReplyGenerateCredential;
	std::function<void(PSS::ErrorCode error, int session_id, const uuid& credential, int account_uid)> OnReplyVerifyCredential;

private:
    // Network message handler type.
    using MessageHandler = std::function<void(const PSS::MessageRoot* message_root)>;

    // ������ ������Ʈ
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

	// TO DO : Manager ������ ������ �ִ� �ٸ� Manager Ŭ���̾�Ʈ�� ����.
};
