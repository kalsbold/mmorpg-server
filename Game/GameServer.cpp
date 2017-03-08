#include "stdafx.h"
#include "GameServer.h"

using namespace Game::Protocol;

void GameServer::Initialize()
{
	size_t thread_count = config_.thread_count;
	ios_pool_ = std::make_shared<IoServicePool>(thread_count);

	// Initialize NetServer
	Configuration config;
	config.io_service_pool = ios_pool_;
	config.max_session_count = config_.max_session_count;
	config.max_receive_buffer_size = 4 * 1024;
	config.min_receive_size = 256;
	config.no_delay = true;

	net_server_ = NetServer::Create(config);
	net_server_->RegisterMessageHandler([this](const Ptr<Session>& session, const uint8_t* buf, size_t bytes)
	{
		// flatbuffer 메시지로 디시리얼라이즈
		const Game::Protocol::NetMessage* net_message = Game::Protocol::GetNetMessage(buf);
		if (net_message == nullptr)
		{
			BOOST_LOG_TRIVIAL(info) << "Invalid NetMessage";
			return;
		}

		auto message_type = net_message->message_type();
		auto it = message_handler_map_.find(message_type);
		
		if (it != message_handler_map_.end())
		{
			// 메시지 핸들러를 실행
			it->second(session, net_message);
		}
		else
		{
			BOOST_LOG_TRIVIAL(info) << "Invalid message_type : " << message_type;
		}
	});

	net_server_->RegisterSessionOpenedHandler([this](const Ptr<Session>& session)
	{
		BOOST_LOG_TRIVIAL(info) << "On session open";

		if (session_opened_handler_)
			session_opened_handler_(session);
	});

	net_server_->RegisterSessionClosedHandler([this](const Ptr<Session>& session, const CloseReason& reason)
	{
		if (reason == CloseReason::ActiveClose)
		{
			BOOST_LOG_TRIVIAL(info) << "On session active close";
		}
		else if (reason == CloseReason::Disconnected)
		{
			BOOST_LOG_TRIVIAL(info) << "On session disconnected";
		}

		if (session_closed_handler_)
			session_closed_handler_(session);
	});

	// Initialize DB
	db_ = std::make_shared<MySQLPool>("127.0.0.1:8413", "nusigmik", "56561163", "Project_MMOG", 4);
}

void GameServer::Run()
{
	Initialize();
	InitializeHandlers();

	std::string bind_address = config_.bind_address;
	uint16_t bind_port = config_.bind_port;
	net_server_->Start(bind_address, bind_port);

	BOOST_LOG_TRIVIAL(info) << "Run Game Server";
	ios_pool_->Wait();
}

void GameServer::Stop()
{
	net_server_->Stop();

	// 종료 작업.


	ios_pool_->Stop();

	BOOST_LOG_TRIVIAL(info) << "Stop Game Server";
}

// Handlers ======================================================================================================
void GameServer::InitializeHandlers()
{
	this->RegisterSessionOpenHandler([this](const Ptr<Session>& session) { OnSessionOpen(session); });
	this->RegisterSessionCloseHandler([this](const Ptr<Session>& session) { OnSessionClose(session); });

	this->RegisterMessageHandler(MessageT_JoinRequest, std::bind(&GameServer::OnJoinRequest, this, std::placeholders::_1, std::placeholders::_2));
	this->RegisterMessageHandler(MessageT_LoginRequest, std::bind(&GameServer::OnLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
}

void GameServer::OnSessionOpen(const Ptr<Session>& session)
{

}

void GameServer::OnSessionClose(const Ptr<Session>& session)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);
	
	// 로그인된 GameSession 을 찾는다.
	auto iter = game_session_map_.find(session->ID());
	if (iter == game_session_map_.end())
		return;

	auto info = iter->second->GetAccountInfo();
	BOOST_LOG_TRIVIAL(info) << "Logout " << info.acc_name << " Session ID : " << session->ID();

	// 삭제
	game_session_map_.erase(iter);
}

void GameServer::OnJoinRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage * net_message)
{
	auto msg = static_cast<const JoinRequest*>(net_message->message());
	
	try
	{
		std::stringstream ss;
		ss << "SELECT acc_name FROM account_tb "
			<< "WHERE acc_name='" << msg->acc_name()->c_str() << "'";

		auto result_set = db_->Excute(ss.str());
		// 이미 있는 계정명. 실패
		if (result_set->rowsCount() > 0)
		{
			JoinFailedReply(session, ErrorCode_JOIN_ACC_NAME_ALREADY);
			return;
		}

		ss.str(""); // 비우기
		ss << "INSERT INTO account_tb (acc_name, password) "
			<< "VALUES ('" << msg->acc_name()->c_str() << "','" << msg->password()->c_str() << "')";

		db_->Excute(ss.str());

		BOOST_LOG_TRIVIAL(info) << "Join success " << msg->acc_name()->c_str();
		JoinSuccessReply(session);
	}
	catch (sql::SQLException& e)
	{
		BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what();
		//	<< ", (MySQL error code : " << e.getErrorCode()
		//	<< ", SQLState: " << e.getSQLState() << " )";
		//std::cout << " (MySQL error code: " << e.getErrorCode();
		//std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;

		JoinFailedReply(session, ErrorCode_FATAL_ERROR);
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
		JoinFailedReply(session, ErrorCode_FATAL_ERROR);
	}
}

void GameServer::JoinSuccessReply(const Ptr<Session>& session)
{
	flatbuffers::FlatBufferBuilder builder;
	auto reply = CreateJoinSuccessReply(builder);
	auto message = CreateNetMessage(builder, MessageT_JoinSuccessReply, reply.Union());
	builder.Finish(message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

void GameServer::JoinFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code)
{
	flatbuffers::FlatBufferBuilder builder;
	auto reply = CreateJoinFailedReply(builder, error_code);
	auto message = CreateNetMessage(builder, MessageT_JoinFailedReply, reply.Union());
	builder.Finish(message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

void GameServer::OnLoginRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage * net_message)
{
	auto msg = static_cast<const LoginRequest*>(net_message->message());

	std::lock_guard<std::mutex> lock_guard(mutex_);

	// 세션이 이미 로그인 되있는지 검사
	if (game_session_map_.find(session->ID()) != game_session_map_.end())
	{
		LoginSuccessReply(session, boost::uuids::to_string(session->ID()));
		return;
	}

	string acc_name = msg->acc_name()->str();
	string password = msg->password()->str();

	std::stringstream ss;
	ss << "SELECT id, acc_name FROM account_tb "
		<< "WHERE acc_name='" << acc_name << "' AND "
		<< "password='" << password << "'";
	
	try
	{
		auto result_set = db_->Excute(ss.str());
		
		// 결과가 없다. 실패
		if (result_set->rowsCount() == 0)
		{
			LoginFailedReply(session, ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
			return;
		}
		result_set->next();

		int acc_id = result_set->getInt(1);
		// 다른 곳에서 이미 로그인된 계정인지 검사.
		auto iter = std::find_if(game_session_map_.begin(), game_session_map_.end(), [acc_id](const std::pair<uuid, Ptr<GameSession>>& pair)
		{
			auto info = pair.second->GetAccountInfo();
			return info.id == acc_id;
		});
		// 이미 로그인됨. 실패
		if (iter != game_session_map_.end())
		{
			LoginFailedReply(session, ErrorCode_LOGIN_ALREADY);
			return;
		}

		// 로그인 성공
		// GameSession 객체를 만들고 맵에 추가
		AccountInfo acc_info;
		acc_info.id = result_set->getInt(1);
		acc_info.acc_name = result_set->getString(2).c_str();
		uuid session_id = session->ID();
		auto game_session = std::make_shared<GameSession>(session, acc_info);
		game_session_map_.insert(std::make_pair(session_id, game_session));
		BOOST_LOG_TRIVIAL(info) << "Login success " << acc_info.acc_name << " Session ID : " << session->ID();

		LoginSuccessReply(session, boost::uuids::to_string(session->ID()));
	}
	catch (sql::SQLException& e)
	{
		BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what();
		//std::cout << " (MySQL error code: " << e.getErrorCode();
		//std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
		
		LoginFailedReply(session, ErrorCode_FATAL_ERROR);
	}
}

void GameServer::LoginSuccessReply(const Ptr<Session>& session, const std::string& session_id)
{
	flatbuffers::FlatBufferBuilder builder;
	auto login_reply = CreateLoginSuccessReply(builder, builder.CreateString(boost::uuids::to_string(session->ID())));
	auto reply_message = CreateNetMessage(builder, MessageT_LoginSuccessReply, login_reply.Union());
	builder.Finish(reply_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

void GameServer::LoginFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code)
{
	flatbuffers::FlatBufferBuilder builder;
	auto login_reply = CreateLoginFailedReply(builder, error_code);
	auto reply_message = CreateNetMessage(builder, MessageT_LoginFailedReply, login_reply.Union());
	builder.Finish(reply_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}


