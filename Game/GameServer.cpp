#include "stdafx.h"
#include "GameServer.h"
#include "ServerConfig.h"

using namespace Game::Protocol;

void GameServer::Run()
{
	ServerConfig& server_config = ServerConfig::GetInstance();

	// ios pool 생성
	size_t thread_count = server_config.thread_count;
	ios_pool_ = std::make_shared<IoServicePool>(thread_count);

	// NetServer Config
	Configuration config;
	config.io_service_pool = ios_pool_;
	config.max_session_count = server_config.max_session_count;
	config.max_receive_buffer_size = 4 * 1024;
	config.min_receive_size = 256;
	config.no_delay = true;

	// Initialize NetServer
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
			BOOST_LOG_TRIVIAL(info) << "Can not find the message handler message_type " << message_type;
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
	db_ = std::make_shared<MySQLPool>(
		server_config.db_host,
		server_config.db_user,
		server_config.db_password,
		server_config.db_schema,
		server_config.db_connection_pool);

	// 네트워크 이벤트 핸들러들 등록
	InitializeHandlers();

	// NetServer 를 시작시킨다.
	std::string bind_address = server_config.bind_address;
	uint16_t bind_port = server_config.bind_port;
	net_server_->Start(bind_address, bind_port);

	BOOST_LOG_TRIVIAL(info) << "Run Game Server";

	// 종료될때 까지 대기
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

	this->RegisterMessageHandler(MessageT_JoinRequest,       std::bind(&GameServer::OnJoinRequest, this, std::placeholders::_1, std::placeholders::_2));
	this->RegisterMessageHandler(MessageT_LoginRequest,      std::bind(&GameServer::OnLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
	this->RegisterMessageHandler(MessageT_CreateHeroRequest, std::bind(&GameServer::OnCreateHeroRequest, this, std::placeholders::_1, std::placeholders::_2));
	this->RegisterMessageHandler(MessageT_HeroListRequest,   std::bind(&GameServer::OnHeroListRequest, this, std::placeholders::_1, std::placeholders::_2));
	this->RegisterMessageHandler(MessageT_DeleteHeroRequest, std::bind(&GameServer::OnDeleteHeroRequest, this, std::placeholders::_1, std::placeholders::_2));
}

void GameServer::OnSessionOpen(const Ptr<Session>& session)
{

}

void GameServer::OnSessionClose(const Ptr<Session>& session)
{
	// 로그인된 GameUser 을 찾는다.
	auto user = FindGameUserSession(session->ID());
	if (!user)
		return;

	auto info = user->GetAccountInfo();
	BOOST_LOG_TRIVIAL(info) << "Logout " << info.acc_name << " Session ID : " << session->ID();

	// 삭제
	RemoveGameUserSession(session->ID());
}

// Join ================================================================================================================
void GameServer::OnJoinRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage * net_message)
{
	auto msg = static_cast<const JoinRequest*>(net_message->message());
	
	try
	{
		const char* acc_name = msg->acc_name()->c_str();
		// 문자열 검사
		std::regex pattern(R"([^A-Za-z0-9_]+)");
		std::cmatch m;
		if (std::regex_search(acc_name, m, pattern))
		{
			CreateHeroFailedReply(session, ErrorCode_INVALID_STRING);
			return;
		}

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
		BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
			<< ", (MySQL error code : " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLState() << " )";

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

// Login ================================================================================================================
void GameServer::OnLoginRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage * net_message)
{
	auto msg = static_cast<const LoginRequest*>(net_message->message());

	// 세션이 이미 로그인 되있는지 찾는다.
	auto user = FindGameUserSession(session->ID());
	if (user)
	{
		// 이미 로그인 되있으면 성공으로 친다.
		// TO DO : 실패로 해야 하나?
		LoginSuccessReply(session, boost::uuids::to_string(session->ID()));
		return;
	}

	string acc_name = msg->acc_name()->str();
	string password = msg->password()->str();

	std::stringstream ss;
	ss << "SELECT id, acc_name, password FROM account_tb "
		<< "WHERE acc_name='" << acc_name << "' AND password='" << password << "'";
	
	try
	{
		auto result_set = db_->Excute(ss.str());
		
		// 결과가 없다. 실패
		if (!result_set->next())
		{
			LoginFailedReply(session, ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
			return;
		}
		
		std::string password2 = result_set->getString("password").c_str();
		// 패스워드가 다르다. 실패
		if (password != password2)
		{
			LoginFailedReply(session, ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
			return;
		}

		int acc_id = result_set->getInt("id");
		// 다른 세션에서 이미 로그인된 계정인지 검사.
		auto user = FindGameUserSession(acc_id);
		// 이미 로그인됨. 실패
		if (user)
		{
			LoginFailedReply(session, ErrorCode_LOGIN_ALREADY);
			return;
		}

		// 로그인 성공
		// GameUser 객체를 만들고 맵에 추가
		AccountInfo acc_info;
		acc_info.id = acc_id;
		acc_info.acc_name = result_set->getString("acc_name").c_str();
		uuid session_id = session->ID();
		auto game_user = std::make_shared<GameUser>(session, acc_info);
		AddGameUserSession(session_id, game_user);
		BOOST_LOG_TRIVIAL(info) << "Login success " << acc_info.acc_name << " Session ID : " << session->ID();

		LoginSuccessReply(session, boost::uuids::to_string(session->ID()));
	}
	catch (sql::SQLException& e)
	{
		BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
			<< ", (MySQL error code : " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLState() << " )";
		
		LoginFailedReply(session, ErrorCode_FATAL_ERROR);
	}
}

void GameServer::LoginSuccessReply(const Ptr<Session>& session, const std::string& session_id)
{
	flatbuffers::FlatBufferBuilder builder;
	auto reply = CreateLoginSuccessReply(builder,
		builder.CreateString(boost::uuids::to_string(session->ID())));

	auto net_message = CreateNetMessage(builder, MessageT_LoginSuccessReply, reply.Union());
	builder.Finish(net_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

void GameServer::LoginFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code)
{
	flatbuffers::FlatBufferBuilder builder;
	auto reply = CreateLoginFailedReply(builder,
		error_code);

	auto net_message = CreateNetMessage(builder, MessageT_LoginFailedReply, reply.Union());
	builder.Finish(net_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

// Create Hero ================================================================================================================
void GameServer::OnCreateHeroRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage * net_message)
{
	auto msg = static_cast<const CreateHeroRequest*>(net_message->message());
	
	auto game_session = FindGameUserSession(session->ID());
	if (!game_session)
	{
		CreateHeroFailedReply(session, ErrorCode_INVALID_SESSION);
		return;
	}

	try
	{
		const char* hero_name = msg->name()->c_str();
		int class_type = msg->class_type();
		
		// 문자열 검사
		std::regex pattern(R"([^A-Za-z0-9_]+)");
		std::cmatch m;
		if (std::regex_search(hero_name, m, pattern))
		{
			CreateHeroFailedReply(session, ErrorCode_INVALID_STRING);
			return;
		}

		std::stringstream ss;
		// 이름 중복 쿼리
		ss << "SELECT name FROM user_hero_tb "
			<< "WHERE name='" << hero_name << "'";

		auto result_set = db_->Excute(ss.str());
		// 이미 있는 이름. 실패
		if (result_set->rowsCount() > 0)
		{
			CreateHeroFailedReply(session, ErrorCode_CREATE_HERO_NAME_ALREADY);
			return;
		}

		auto acc_info = game_session->GetAccountInfo();

		ss.str(""); // 비우기
		// Insert
		ss << "INSERT INTO user_hero_tb (acc_id, name, class_type) "
			<< "VALUES (" << acc_info.id << ",'" << hero_name << "','" << class_type << "')";
		db_->Excute(ss.str());

		ss.str(""); // 비우기
		// Insert 후 생성된 row 조회 (다른 방법이 있을거다?)
		ss << "SELECT id, name, class_type, level FROM user_hero_tb "
			<< "WHERE acc_id=" << acc_info.id << " and name='" << hero_name << "'";
		
		result_set = db_->Excute(ss.str());
		if (!result_set->next())
		{
			// 생성 된게 없으면 실패
			CreateHeroFailedReply(session, ErrorCode_CREATE_HERO_CANNOT_CREATE);
			return;
		}

		// 생성 성공
		int id = result_set->getInt(1);
		int level = result_set->getInt(4);
		CreateHeroSuccessReply(session, id, hero_name, class_type, level);
		BOOST_LOG_TRIVIAL(info) << "Create Hero success : " << hero_name;
	}
	catch (sql::SQLException& e)
	{
		BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
			<< ", (MySQL error code : " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLState() << " )";

		CreateHeroFailedReply(session, ErrorCode_FATAL_ERROR);
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
		CreateHeroFailedReply(session, ErrorCode_FATAL_ERROR);
	}
}

void GameServer::CreateHeroSuccessReply(const Ptr<Session>& session, int id, const char* name, int class_type, int level)
{
	flatbuffers::FlatBufferBuilder builder;
	auto hero = CreateHeroInfoSimple(builder,
		id,
		builder.CreateString(name),
		class_type,
		level);
	auto reply = CreateCreateHeroSuccessReply(builder, hero);

	auto net_message = CreateNetMessage(builder, MessageT_CreateHeroSuccessReply, reply.Union());
	builder.Finish(net_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

void GameServer::CreateHeroFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code)
{
	flatbuffers::FlatBufferBuilder builder;
	auto reply = CreateCreateHeroFailedReply(builder,
		error_code);

	auto net_message = CreateNetMessage(builder, MessageT_CreateHeroFailedReply, reply.Union());
	builder.Finish(net_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

// Hero List ================================================================================================================
void GameServer::OnHeroListRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage * net_message)
{
	auto msg = static_cast<const CreateHeroRequest*>(net_message->message());

	auto game_session = FindGameUserSession(session->ID());
	if (!game_session)
	{
		return;
	}

	try
	{
		std::stringstream ss;
		ss << "SELECT id, name, class_type, level FROM user_hero_tb "
			<< "WHERE acc_id=" << game_session->GetAccountInfo().id
			<< " AND del_type='F'";
		
		std::vector<HeroSimpleData> hero_list;

		auto result_set = db_->Excute(ss.str());
		while (result_set->next())
		{
			/*
			HeroSimpleData hero_data;
			hero_data.id = result_set->getInt(1);
			hero_data.name = result_set->getString(2).c_str();
			hero_data.class_type = (ClassType)result_set->getInt(3);
			hero_data.level = result_set->getInt(4);
			hero_list.push_back(hero_data);
			*/
			hero_list.push_back(
				HeroSimpleData{
					result_set->getInt(1),
					result_set->getString(2).c_str(),
					(ClassType)result_set->getInt(3),
					result_set->getInt(4)
				});
		}

		HeroListReply(session, hero_list);
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

void GameServer::HeroListReply(const Ptr<Session>& session, const std::vector<HeroSimpleData>& hero_list)
{
	flatbuffers::FlatBufferBuilder builder;

	std::vector<flatbuffers::Offset<HeroInfoSimple>> hero_vector;
	
	for (auto& hero_data : hero_list)
	{
		auto hero_info_simple = CreateHeroInfoSimple(builder,
			hero_data.id,
			builder.CreateString(hero_data.name),
			(int)hero_data.class_type,
			hero_data.level);

		hero_vector.push_back(hero_info_simple);
	}
	auto reply = CreateHeroListReply(builder, builder.CreateVector(hero_vector));

	auto net_message = CreateNetMessage(builder, MessageT_HeroListReply, reply.Union());
	builder.Finish(net_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

// Delete Hero ==========================================================================================================
void GameServer::OnDeleteHeroRequest(const Ptr<Session>& session, const Game::Protocol::NetMessage * net_message)
{
	auto msg = static_cast<const DeleteHeroRequest*>(net_message->message());

	auto game_session = FindGameUserSession(session->ID());
	if (!game_session)
	{
		CreateHeroFailedReply(session, ErrorCode_INVALID_SESSION);
		return;
	}

	try
	{
		const int hero_id = msg->hero_id();
		auto acc_info = game_session->GetAccountInfo();

		std::stringstream ss;
		// del_type 을 'T' 로 업데이트. 실제로 지우지는 않는다.
		ss << "UPDATE user_hero_tb SET "
			<< "del_type='T' "
			<< "WHERE id=" << hero_id <<" AND acc_id=" << acc_info.id;

		db_->Excute(ss.str());

		// 성공
		DeleteHeroSuccessReply(session, hero_id);
		BOOST_LOG_TRIVIAL(info) << "Delete Hero success. Hero Id:" << hero_id;
	}
	catch (sql::SQLException& e)
	{
		BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
			<< ", (MySQL error code : " << e.getErrorCode()
			<< ", SQLState: " << e.getSQLState() << " )";

		DeleteHeroFailedReply(session, ErrorCode_FATAL_ERROR);
	}
	catch (std::exception& e)
	{
		BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
		DeleteHeroFailedReply(session, ErrorCode_FATAL_ERROR);
	}
}

void GameServer::DeleteHeroSuccessReply(const Ptr<Session>& session, int hero_id)
{
	flatbuffers::FlatBufferBuilder builder;
	auto reply = CreateDeleteHeroSuccessReply(builder, hero_id);

	auto net_message = CreateNetMessage(builder, MessageT_DeleteHeroSuccessReply, reply.Union());
	builder.Finish(net_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

void GameServer::DeleteHeroFailedReply(const Ptr<Session>& session, Game::Protocol::ErrorCode error_code)
{
	flatbuffers::FlatBufferBuilder builder;
	auto reply = CreateDeleteHeroFailedReply(builder,
		error_code);

	auto net_message = CreateNetMessage(builder, MessageT_DeleteHeroFailedReply, reply.Union());
	builder.Finish(net_message);

	session->Send(builder.GetBufferPointer(), builder.GetSize());
}

