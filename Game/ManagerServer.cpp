#include "stdafx.h"
#include "ManagerServer.h"
#include "Settings.h"
#include "DBSchema.h"
#include "protocol_ss_helper.h"

namespace fb = flatbuffers;
namespace db = db_schema;
using namespace PSS::Manager;

ManagerServer::ManagerServer()
{
}

ManagerServer::~ManagerServer()
{
	Stop();
}

void ManagerServer::Run()
{
	Settings& settings = Settings::GetInstance();

	SetName(settings.name);

	// Create io_service loop
	size_t thread_count = settings.thread_count;
	ios_loop_ = std::make_shared<net::IoServiceLoop>(thread_count);
	
	// NetServer Config
	net::ServerConfig net_config;
	net_config.io_service_loop = ios_loop_;
	net_config.max_session_count = settings.max_session_count;
	net_config.max_receive_buffer_size = settings.max_receive_buffer_size;
	net_config.min_receive_size = settings.min_receive_size;
	net_config.no_delay = settings.no_delay;
	// Create NetServer
	net_server_ = net::NetServer::Create(net_config);
	net_server_->RegisterSessionOpenedHandler([this](auto& session) { HandleSessionOpened(session); });
	net_server_->RegisterSessionClosedHandler([this](auto& session, auto reason) { HandleSessionClosed(session, reason); });
	net_server_->RegisterMessageHandler([this](auto& session, auto buf, auto bytes) { HandleMessage(session, buf, bytes); });

	// �޽��� �ڵ鷯 ���
	RegisterHandlers();

	// Create DB connection Pool
	db_conn_ = MySQLPool::Create(
		settings.db_host,
		settings.db_user,
		settings.db_password,
		settings.db_schema,
		settings.db_connection_pool);

	// Frame Update ����.
	strand_ = std::make_shared<boost::asio::strand>(ios_loop_->GetIoService());
	update_timer_ = std::make_shared<timer>(strand_->get_io_service());
	ScheduleNextUpdate(clock_type::now(), TIME_STEP);

	// NetServer �� ���۽�Ų��.
	std::string bind_address = settings.bind_address;
	uint16_t bind_port = settings.bind_port;
	net_server_->Start(bind_address, bind_port);

	BOOST_LOG_TRIVIAL(info) << "Run " << GetName();
}

void ManagerServer::Stop()
{
	// ���� �۾�.
	net_server_->Stop();
	ios_loop_->Stop();

	BOOST_LOG_TRIVIAL(info) << "Stop " << GetName();
}

// ������ ����� ������ ���
void ManagerServer::Wait()
{
	if (ios_loop_)
		ios_loop_->Wait();
}

const Ptr<RemoteManagerClient> ManagerServer::GetRemoteClient(int session_id)
{
	auto iter = remote_clients_.find(session_id);

	return iter == remote_clients_.end() ? nullptr : iter->second;
}

const Ptr<RemoteManagerClient> ManagerServer::GetRemoteClientByName(const std::string & name)
{
	auto iter = std::find_if(remote_clients_.begin(), remote_clients_.end(), [name](auto& var)
	{
		return (var.second->server_name == name);
	});

	return iter == remote_clients_.end() ? nullptr : iter->second;
}


// ������ ������Ʈ
void ManagerServer::DoUpdate(double delta_time)
{
	// ���� ���� �ð�.
	constexpr duration wait_time(60000ms);

	std::lock_guard<std::mutex> lock_guard(mutex_);

	// �α׾ƿ��� �ð��� ������ �����.
	// TO DO : �����ϰ� ��ü�� ��������. �����ϴ� ����� ������.
	auto iter = user_session_set_.begin();
	for (; iter != user_session_set_.end(); )
	{
		if (!iter->login_ && iter->logout_time_ + wait_time < clock_type::now())
		{
			BOOST_LOG_TRIVIAL(info) << "Delete user session : " << iter->account_uid_ << " " << iter->credential_;
			iter = user_session_set_.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

void ManagerServer::AddRemoteClient(int session_id, Ptr<RemoteManagerClient> remote_client)
{
	remote_clients_.emplace(session_id, remote_client);
}

void ManagerServer::RemoveRemoteClient(int session_id)
{
	remote_clients_.erase(session_id);
}

void ManagerServer::RemoveRemoteClient(const Ptr<RemoteManagerClient>& remote_client)
{
	remote_clients_.erase(remote_client->GetSessionID());
}

void ManagerServer::ProcessRemoteClientDisconnected(const Ptr<RemoteManagerClient>& rc)
{
	// ��Ͽ��� ����
	RemoveRemoteClient(rc);
	rc->OnDisconnected();

	BOOST_LOG_TRIVIAL(info) << "Manager server logout : " << rc->server_name;
}

void ManagerServer::ScheduleNextUpdate(const time_point& now, const duration& timestep)
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

void ManagerServer::NotifyUnauthedAccess(const Ptr<net::Session>& session)
{
	fb::FlatBufferBuilder fbb;
	auto notify = PSS::CreateNotify_UnauthedAccess(fbb);
	auto msg_root = PSS::CreateMessageRoot(fbb, PSS::MessageType::Notify_UnauthedAccess, notify.Union());
	PSS::FinishMessageRootBuffer(fbb, msg_root);

	session->Send(fbb.GetBufferPointer(), fbb.GetSize());
}

void ManagerServer::HandleMessage(const Ptr<net::Session>& session, const uint8_t* buf, size_t bytes)
{
	// flatbuffer �޽����� ��ø��������
	const auto* message_root = PSS::GetMessageRoot(buf);
	if (message_root == nullptr)
	{
		BOOST_LOG_TRIVIAL(info) << "Invalid MessageRoot";
		return;
	}

	auto message_type = message_root->message_type();
	auto iter = message_handlers_.find(message_type);
	if (iter == message_handlers_.end())
	{
		BOOST_LOG_TRIVIAL(info) << "Can not find the message handler. message_type : " << PSS::EnumNameMessageType(message_type);
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

void ManagerServer::HandleSessionOpened(const Ptr<net::Session>& session)
{

}

void ManagerServer::HandleSessionClosed(const Ptr<net::Session>& session, net::CloseReason reason)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);
	auto remote_client = GetRemoteClient(session->GetID());
	if (!remote_client)
		return;

	ProcessRemoteClientDisconnected(remote_client);
}

// Login ================================================================================================================
void ManagerServer::OnLogin(const Ptr<net::Session>& session, const PSS::Manager::Request_Login* message)
{
	if (message == nullptr) return;

	std::lock_guard<std::mutex> lock_guard(mutex_);
	const std::string name = message->client_name()->str();
	const ServerType type = (ServerType)message->client_type();

	// �̹� �����ưų� ���� �̸��� ������ ����
	if( GetRemoteClient(session->GetID()) || GetRemoteClientByName(name))
	{
		Reply_LoginT reply;
		reply.error_code = PSS::ErrorCode::LOGIN_ALREADY_CONNECTED;
		PSS::Send(*session, reply);
		return;
	}

	// �α��� ����. RemoteClient ���� �� �߰�.
	auto new_rc = std::make_shared<RemoteManagerClient>(session);
	new_rc->server_name = name;
	new_rc->server_type = type;
	AddRemoteClient(new_rc->GetSessionID(), new_rc);

	BOOST_LOG_TRIVIAL(info) << "Manager server login : " << new_rc->server_name;

	Reply_LoginT reply;
	reply.error_code = PSS::ErrorCode::OK;
	PSS::Send(*session, reply);
}

void ManagerServer::OnGenerateCredential(const Ptr<net::Session>& session, const PSS::Manager::Request_GenerateCredential * message)
{
	if (message == nullptr) return;

	std::lock_guard<std::mutex> lock_guard(mutex_);
	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	const int account_uid = message->account_uid();
	const uuid new_credential = boost::uuids::random_generator()();

	auto& indexer = user_session_set_.get<tags::account_uid>();
	auto iter = indexer.find(account_uid);
	if (iter != indexer.end())
	{
		// credential �� ���� �߱� �ϰ� �α��� ���·� ������.
		indexer.modify(iter, [&new_credential](UserSession& var) { var.credential_ = new_credential; var.login_ = true; });
	}
	else
	{
		// �� ����
		user_session_set_.insert(UserSession(account_uid, new_credential));
	}
	// �ٽ� ã�´�.
	iter = indexer.find(account_uid);
	if (iter == indexer.end())
		return;

	BOOST_LOG_TRIVIAL(info) << "Generate credential. account_uid: " << iter->account_uid_ << " credential: " << iter->credential_;

	Reply_GenerateCredentialT reply;
	reply.session_id = message->session_id();
	reply.credential = boost::uuids::to_string(new_credential);
	PSS::Send(*session, reply);
}

void ManagerServer::OnVerifyCredential(const Ptr<net::Session>& session, const PSS::Manager::Request_VerifyCredential * message)
{
	if (message == nullptr) return;

	std::lock_guard<std::mutex> lock_guard(mutex_);
	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	const uuid credential = boost::uuids::string_generator()(message->credential()->c_str());
	auto error_code = PSS::ErrorCode::OK;

	auto& indexer = user_session_set_.get<tags::credential>();
	auto iter = indexer.find(credential);
	if (iter == indexer.end())
	{
		// ������ ����
		Reply_VerifyCredentialT reply;
		reply.session_id = message->session_id();
		reply.error_code = PSS::ErrorCode::VERIFY_CREDENTIAL_FAILED;
		PSS::Send(*session, reply);
		return;
	}

	// �ٽ� �α��� ���·� ������.
	indexer.modify(iter, [](UserSession& user) { user.login_ = true; });

	BOOST_LOG_TRIVIAL(info) << "Verify credential. account_uid: " << iter->account_uid_ << " credential: " << iter->credential_;

	Reply_VerifyCredentialT reply;
	reply.session_id = message->session_id();
	reply.error_code = PSS::ErrorCode::OK;
	PSS::Send(*session, reply);
}

void ManagerServer::OnUserLogout(const Ptr<net::Session>& session, const PSS::Manager::Notify_UserLogout * message)
{
	if (message == nullptr) return;

	std::lock_guard<std::mutex> lock_guard(mutex_);
	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	// �ٷ� ������ �ʰ� �α׾ƿ� ���·� ����.
	const int account_uid = message->account_uid();
	auto& indexer = user_session_set_.get<tags::account_uid>();
	auto iter = indexer.find(account_uid);
	if (iter != indexer.end() && iter->login_)
	{
		indexer.modify(iter, [](UserSession& user) { user.login_ = false; user.logout_time_ = clock_type::now(); });
		BOOST_LOG_TRIVIAL(info) << "User logout. account_uid: " << iter->account_uid_ << " credential: " << iter->credential_;
	}
}

void ManagerServer::RegisterHandlers()
{
	RegisterMessageHandler<PSS::Manager::Request_Login>([this](auto& session, auto* msg) { OnLogin(session, msg); });
	RegisterMessageHandler<PSS::Manager::Request_GenerateCredential>([this](auto& session, auto* msg) { OnGenerateCredential(session, msg); });
	RegisterMessageHandler<PSS::Manager::Request_VerifyCredential>([this](auto& session, auto* msg) { OnVerifyCredential(session, msg); });
	RegisterMessageHandler<PSS::Manager::Notify_UserLogout>([this](auto& session, auto* msg) { OnUserLogout(session, msg); });
}