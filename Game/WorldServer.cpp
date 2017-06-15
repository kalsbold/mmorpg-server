#include "stdafx.h"
#include "WorldServer.h"
#include "RemoteWorldClient.h"
#include "ManagerClient.h"
#include "Settings.h"
#include "DBSchema.h"
#include "StaticCachedData.h"
#include "protocol_ss_helper.h"
#include "protocol_cs_helper.h"

namespace fb = flatbuffers;
namespace db = db_schema;

WorldServer::WorldServer()
{
}

WorldServer::~WorldServer()
{
	Stop();
}

void WorldServer::Run()
{
	Settings& settings = Settings::GetInstance();

	SetName(settings.name);

	// Create io_service loop
	size_t thread_count = settings.thread_count;
	ios_loop_ = std::make_shared<net::IoServiceLoop>(thread_count);
	// NetServer Config
	net::ServerConfig server_config;
	server_config.io_service_loop = ios_loop_;
	server_config.max_session_count = settings.max_session_count;
	server_config.max_receive_buffer_size = settings.max_receive_buffer_size;
	server_config.min_receive_size = settings.min_receive_size;
	server_config.no_delay = settings.no_delay;
	// Create NetServer
	net_server_ = net::NetServer::Create(server_config);
	net_server_->RegisterSessionOpenedHandler([this](auto& session) { HandleSessionOpened(session); });
	net_server_->RegisterSessionClosedHandler([this](auto& session, auto reason) { HandleSessionClosed(session, reason); });
	net_server_->RegisterMessageHandler([this](auto& session, auto* buf, auto bytes) { HandleMessage(session, buf, bytes); });

	// �ڵ鷯 ���
	RegisterHandlers();

	// Create DB connection Pool
	db_conn_ = MySQLPool::Create(
		settings.db_host,
		settings.db_user,
		settings.db_password,
		settings.db_schema,
		settings.db_connection_pool);

	// �ʿ��� ������ �ε�.
	CharacterAttributeTable::Load(db_conn_);
	MapTable::Load(db_conn_);

	// Frame Update ����.
	strand_ = std::make_shared<boost::asio::strand>(ios_loop_->GetIoService());
	update_timer_ = std::make_shared<timer>(strand_->get_io_service());
	ScheduleNextUpdate(clock_type::now(), TIME_STEP);

	// NetServer �� ���۽�Ų��.
	std::string bind_address = settings.bind_address;
	uint16_t bind_port = settings.bind_port;
	net_server_->Start(bind_address, bind_port);

	// Manager ������ ���� �ϴ� Ŭ���̾�Ʈ.
	net::ClientConfig client_config;
	client_config.io_service_loop = ios_loop_;
	client_config.min_receive_size = settings.min_receive_size;
	client_config.max_receive_buffer_size = settings.max_receive_buffer_size;
	client_config.no_delay = settings.no_delay;
	manager_client_ = std::make_shared<ManagerClient>(client_config, this, ServerType::Login_Server, GetName());
	// Ŭ���̾�Ʈ �� �޽��� �ڵ鷯 ���.
	RegisterManagerClientHandlers();
	// Manager ������ ���� ����.
	manager_client_->Connect(settings.manager_address, settings.manager_port);

	BOOST_LOG_TRIVIAL(info) << "Run " << GetName();
}

void WorldServer::Stop()
{
	// ���� �۾�.
	net_server_->Stop();
	ios_loop_->Stop();

	BOOST_LOG_TRIVIAL(info) << "Stop " << GetName();
}

// ������ ����� ������ ���
void WorldServer::Wait()
{
	if (ios_loop_)
		ios_loop_->Wait();
}

const Ptr<RemoteWorldClient> WorldServer::GetRemoteClient(int session_id)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);
	auto iter = remote_clients_.find(session_id);

	return iter != remote_clients_.end() ? iter->second : nullptr;
}

const Ptr<RemoteWorldClient> WorldServer::GetRemoteClientByAccountUID(int account_uid)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);
	auto iter = std::find_if(remote_clients_.begin(), remote_clients_.end(), [account_uid](auto& var)
	{
		return (var.second->GetAccount() && var.second->GetAccount()->uid == account_uid);
	});

	return iter != remote_clients_.end() ? iter->second : nullptr;
}

const Ptr<RemoteWorldClient> WorldServer::GetAuthedRemoteClient(int session_id)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);
	auto iter = remote_clients_.find(session_id);

	return ((iter != remote_clients_.end()) && iter->second->IsAuthenticated()) ? iter->second : nullptr;
}

void WorldServer::AddRemoteClient(int session_id, Ptr<RemoteWorldClient> remote_client)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);
	remote_clients_.emplace(session_id, remote_client);
}

void WorldServer::RemoveRemoteClient(int session_id)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);
	remote_clients_.erase(session_id);
}

void WorldServer::RemoveRemoteClient(const Ptr<RemoteWorldClient>& remote_client)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);
	remote_clients_.erase(remote_client->GetSessionID());
}

void WorldServer::ProcessRemoteClientDisconnected(const Ptr<RemoteWorldClient>& rc)
{
	// ��Ͽ��� ����
	RemoveRemoteClient(rc);
	rc->OnDisconnected();

	// Manager ������ �α׾ƿ��� �˸���.
	manager_client_->NotifyUserLogout(rc->GetAccount()->uid);
	BOOST_LOG_TRIVIAL(info) << "Logout. account_uid: " << rc->GetAccount()->uid << " user_name: " << rc->GetAccount()->user_name;
}

void WorldServer::ScheduleNextUpdate(const time_point& now, const duration& timestep)
{
	auto update_time = now + timestep;
	update_timer_->expires_at(update_time);
	update_timer_->async_wait(strand_->wrap([this, start_time = now, timestep](auto& error)
	{
		if (error) return;

		auto now = clock_type::now();
		double delta_time = double_seconds(now - start_time).count();
		ScheduleNextUpdate(now, timestep);
		DoUpdate(delta_time);
	}));
}

void WorldServer::NotifyUnauthedAccess(const Ptr<net::Session>& session)
{
	fb::FlatBufferBuilder fbb;
	auto notify = PCS::CreateNotify_UnauthedAccess(fbb);
	auto root = PCS::CreateMessageRoot(fbb, PCS::MessageType::Notify_UnauthedAccess, notify.Union());
	FinishMessageRootBuffer(fbb, root);

	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

void WorldServer::HandleMessage(const Ptr<net::Session>& session, const uint8_t* buf, size_t bytes)
{
	// flatbuffer �޽����� ��ø��������
	const auto* message_root = PCS::GetMessageRoot(buf);
	if (message_root == nullptr)
	{
		BOOST_LOG_TRIVIAL(info) << "Invalid MessageRoot";
		return;
	}

	auto message_type = message_root->message_type();
	auto iter = message_handlers_.find(message_type);
	if (iter == message_handlers_.end())
	{
		BOOST_LOG_TRIVIAL(info) << "Can not find the message handler. message_type : " << PCS::EnumNameMessageType(message_type);
		return;
	}

	try
	{
		// �޽��� �ڵ鷯�� ����
		iter->second(session, message_root);
	}
	catch (sql::SQLException& e)
	{
		BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
			<< ", (MySQL error code : " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLState() << " )";
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
	}
}

void WorldServer::HandleSessionOpened(const Ptr<net::Session>& session)
{

}

void WorldServer::HandleSessionClosed(const Ptr<net::Session>& session, net::CloseReason reason)
{
	auto remote_client = GetRemoteClient(session->GetID());
	if (!remote_client)
		return;

	ProcessRemoteClientDisconnected(remote_client);
}

// Login ================================================================================================================
void WorldServer::OnLogin(const Ptr<net::Session>& session, const PCS::Login::Request_Login* message)
{
	if (message == nullptr) return;

	const std::string user_name = message->user_name()->str();
	const std::string password = message->password()->str();

	auto db_account = db::Account::Fetch(db_conn_, user_name);
	// ������ ����.
	if (!db_account)
	{
		PCS::Login::Reply_LoginFailedT reply;
		reply.error_code = PCS::ErrorCode::LOGIN_INCORRECT_ACC_NAME;
		PCS::Send(*session, reply);
		return;
	}
	// ����� �ٸ���.
	if (db_account->password != password)
	{
		PCS::Login::Reply_LoginFailedT reply;
		reply.error_code = PCS::ErrorCode::LOGIN_INCORRECT_ACC_PASSWORD;
		PCS::Send(*session, reply);
		return;
	}

	// TO DO : �л� �������� �ߺ� �α��� ó����?
	//// �ߺ� �α��� ó��.
	//if (auto rc = GetRemoteClientByAccountID(db_account->id))
	//{
	//	PCS::Login::Reply_LoginFailedT reply;
	//	reply.error_code = PCS::ErrorCode::LOGIN_DUPLICATION;

	//	// RemoteClient ������ �����ش�.
	//	rc->Send(reply);
	//	rc->Disconnect();
	//	// ����ó���� �ٷ� ���ش�.
	//	ProcessRemoteClientDisconnected(rc);

	//	// ���� ������ �ƴϾ����� �ű� ���ǵ� ������ �����ش�.
	//	if (rc->GetSession() != session)
	//	{
	//		PCS::Send(session, reply);
	//		session->Close();
	//	}
	//	return;
	//}

	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
	{
		// RemoteWorldClient ���� �� �߰�.
		rc = std::make_shared<RemoteWorldClient>(session, db_account);
		AddRemoteClient(session->GetID(), rc);
	}

	BOOST_LOG_TRIVIAL(info) << "Login. account_uid: " << rc->GetAccount()->uid << " user_name: " << rc->GetAccount()->user_name;

	// Manager ������ ����Ű ��û.
	manager_client_->RequestGenerateCredential(session->GetID(), db_account->uid);
}

void WorldServer::RegisterHandlers()
{
	RegisterMessageHandler<PCS::Login::Request_Join>([this](auto& session, auto* msg) { OnJoin(session, msg); });
	
}

void WorldServer::RegisterManagerClientHandlers()
{
	manager_client_->OnLoginManagerServer = [this](PSS::ErrorCode ec) {
		if (PSS::ErrorCode::OK == ec)
		{
			BOOST_LOG_TRIVIAL(info) << "Manager ���� ���� ����!";
		}
		else
		{
			// Manager ������ ������ �����ϸ� �����Ѵ�.
			BOOST_LOG_TRIVIAL(info) << "Manager ���� ���� ����!";
			Stop();
		}
	};

	manager_client_->OnDisconnectManagerServer = [this]() {
		//  Manager ������ ������ ���� ���� ���� �Ѵ�.
		BOOST_LOG_TRIVIAL(info) << "Manager ������ ������ ���� ��.";
		Stop();
	};

	manager_client_->OnReplyVerifyCredential = [this](PSS::ErrorCode ec, int session_id) {
		auto rc = GetRemoteClient(session_id);
		if (!rc)
			return;

		rc->Authenticate(credential);

		BOOST_LOG_TRIVIAL(info) << "Authenticate. account_uid: " << rc->GetAccount()->uid << " user_name: " << rc->GetAccount()->user_name;

		PCS::Login::Reply_LoginSuccessT reply_msg;
		reply_msg.credential = boost::uuids::to_string(credential);
		PCS::Send(*rc, reply_msg);
	};
}
