#include "stdafx.h"
#include "GameServer.h"
#include "ServerConfig.h"
#include "GamePlayer.h"
#include "DBSchema.h"
#include "AccountManager.h"
#include "StaticCachedData.h"
#include "MessageHelper.h"

namespace mmog {

	using namespace std;
	using namespace flatbuffers;
	using namespace protocol;
	using namespace helper;
	namespace db = db_schema;

	void GameServer::Run()
	{
		ServerConfig& server_config = ServerConfig::GetInstance();
		
		SetName(server_config.name);
		RegisterHandlers();

		// Create io_service pool
		size_t thread_count = server_config.thread_count;
		ios_pool_ = std::make_shared<IoServicePool>(thread_count);
		// NetServer Config
		Configuration net_config;
		net_config.io_service_pool = ios_pool_;
		net_config.max_session_count = server_config.max_session_count;
		net_config.max_receive_buffer_size = server_config.max_receive_buffer_size;
		net_config.min_receive_size = server_config.min_receive_size;
		net_config.no_delay = server_config.no_delay;
		// Create NetServer
		net_server_ = NetServer::Create(net_config);
		net_server_->RegisterMessageHandler(
			[this](const Ptr<Session>& session, const uint8_t* buf, size_t bytes)
			{
				HandleMessage(session, buf, bytes);
			});

		net_server_->RegisterSessionOpenedHandler(
			[this](const Ptr<Session>& session)
			{
				HandleSessionOpened(session);
			});

		net_server_->RegisterSessionClosedHandler(
			[this](const Ptr<Session>& session, const CloseReason& reason)
			{
				HandleSessionClosed(session, reason);
			});

		// NetServer 를 시작시킨다.
		std::string bind_address = server_config.bind_address;
		uint16_t bind_port = server_config.bind_port;
		net_server_->Start(bind_address, bind_port);

		// Create DB connection Pool
		db_conn_ = std::make_shared<MySQLPool>(
			server_config.db_host,
			server_config.db_user,
			server_config.db_password,
			server_config.db_schema,
			server_config.db_connection_pool);
		
		BOOST_LOG_TRIVIAL(info) << "Run Game Server : " << GetName();

		// 종료될때 까지 대기
		ios_pool_->Wait();
	}

	void GameServer::Stop()
	{
		net_server_->Stop();

		// 종료 작업.


		ios_pool_->Stop();

		BOOST_LOG_TRIVIAL(info) << "Stop Game Server : " << GetName();
	}

	inline const Ptr<GamePlayer> GameServer::GetGamePlayer(const SessionID & session_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		auto iter = players_.find(session_id);
		if (iter == players_.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	inline void GameServer::AddGamePlayer(SessionID session_id, Ptr<GamePlayer> user)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		players_.insert(std::make_pair(session_id, user));
	}

	inline void GameServer::RemoveGamePlayer(const SessionID & session_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		players_.erase(session_id);
	}

	void GameServer::HandleMessage(const Ptr<Session>& session, const uint8_t* buf, size_t bytes)
	{
		// flatbuffer 메시지로 디시리얼라이즈
		const NetMessage* net_message = GetNetMessage(buf);
		if (net_message == nullptr)
		{
			BOOST_LOG_TRIVIAL(info) << "Invalid NetMessage";
			return;
		}

		auto message_type = net_message->message_type();
		auto it = message_handlers_.find(message_type);

		if (it == message_handlers_.end())
		{
			BOOST_LOG_TRIVIAL(info) << "Can not find the message handler message_type " << message_type;
			return;
		}

		try
		{
			// 메시지 핸들러를 실행
			it->second(session, net_message);
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

	void GameServer::HandleSessionOpened(const Ptr<Session>& session)
	{

	}

	void GameServer::HandleSessionClosed(const Ptr<Session>& session, CloseReason reason)
	{
		// 로그아웃 처리
		AccountManager::GetInstance().SetLogout(session);

		// 게임중인 플레이어
		auto player = GetGamePlayer(session->GetID());
		if (!player)
			return;

		// 클라이언트 측에서 끊었을 경우 종료 처리
		if (CloseReason::Disconnected == reason)
		{
			player->OnDisconnected();
		}

		// 삭제
		RemoveGamePlayer(session->GetID());
	}

	// Login ================================================================================================================
	void GameServer::OnLogin(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestLogin*>(net_message->message());

		FlatBufferBuilder builder;

		const string acc_name = msg->acc_name()->str();
		const string password = msg->password()->str();

		auto account = db::Account::Fetch(*db_conn_, acc_name);
		if (!account)
		{
			auto response = CreateLoginFailed(builder,
				ErrorCode_LOGIN_INCORRECT_ACC_NAME);
			Send(session, builder, response);
			return;
		}
			
		if (account->password != password)
		{
			auto response = CreateLoginFailed(builder,
				ErrorCode_LOGIN_INCORRECT_ACC_PASSWORD);
			Send(session, builder, response);
			return;
		}

		// 로그인 체크
		if (!AccountManager::GetInstance().CheckAndSetLogin(account->id, session))
		{
			// 로그인 되어있는 세션을 얻는다
			auto logged_in_session = AccountManager::GetInstance().FindSession(account->id);
			// 동일 세션이 아니면
			if(logged_in_session != session)
			{
				// 이전 세션 로그아웃
				AccountManager::GetInstance().SetLogout(account->id);
				// 게임 플레이 종료 처리
				auto player = GetGamePlayer(logged_in_session->GetID());
				player->Disconnect();
				// 새 새션 로그인
				AccountManager::GetInstance().CheckAndSetLogin(account->id, session);
			}
		}

		BOOST_LOG_TRIVIAL(info) << "Login : " << account->id;

		auto response = CreateLoginSuccess(builder,
			builder.CreateString(boost::uuids::to_string(session->GetID())));
		Send(session, builder, response);

		/*
		Account acc_info;
		acc_info.id = acc_id;
		acc_info.acc_name = result_set->getString("acc_name").c_str();
		uuid session_id = session->GetID();
		auto game_user = std::make_shared<GamePlayer>(this, session, acc_info);
		AddGamePlayer(session_id, game_user);
		BOOST_LOG_TRIVIAL(info) << "Login success : " << acc_info.acc_name;
		// 성공
		auto response = CreateLoginSuccess(builder,
			builder.CreateString(boost::uuids::to_string(session->GetID())));
		Send(session, builder, response);
		*/
	}

	// Join ================================================================================================================
	void GameServer::OnJoin(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestJoin*>(net_message->message());

		FlatBufferBuilder builder;
	
		const char* acc_name = msg->acc_name()->c_str();
		const char* password = msg->password()->c_str();
		// 문자열 검사
		std::regex pattern(R"([^A-Za-z0-9_]+)");
		std::cmatch m;
		if (std::regex_search(acc_name, m, pattern))
		{
			auto response = CreateJoinFailed(builder, ErrorCode_INVALID_STRING);
			Send(session, builder, response);
			return;
		}

		if (db::Account::Fetch(*db_conn_, acc_name))
		{
			auto response = CreateJoinFailed(builder, ErrorCode_JOIN_ACC_NAME_ALREADY);
			Send(session, builder, response);
			return;
		}

		if (!db::Account::Create(*db_conn_, acc_name, password))
		{
			auto response = CreateJoinFailed(builder, ErrorCode_JOIN_CANNOT_ACC_CREATE);
			Send(session, builder, response);
			return;
		}

		BOOST_LOG_TRIVIAL(info) << "Join : " << acc_name;

		auto response = CreateJoinSuccess(builder);
		Send(session, builder, response);
	}

	// Create Character ================================================================================================================
	void GameServer::OnCreateCharacter(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestCreateCharacter*>(net_message->message());
		
		// 로그인 체크
		int account_id = AccountManager::GetInstance().FindAccount(session);
		if (account_id == 0)
		{
			CreateCharacterFailedT response;
			response.error_code = ErrorCode_INVALID_SESSION;
			Send(session, response);
			return;
		}

		const char* character_name = msg->name()->c_str();
		const int class_type = msg->class_type();
		// 문자열 검사
		std::regex pattern(R"([^A-Za-z0-9_]+)");
		std::cmatch m;
		if (std::regex_search(character_name, m, pattern))
		{
			CreateCharacterFailedT response;
			response.error_code = ErrorCode_INVALID_STRING;
			Send(session, response);
			return;
		}

		if (db::Character::Fetch(*db_conn_, character_name))
		{
			// 이미 있는 이름.
			CreateCharacterFailedT response;
			response.error_code = ErrorCode_CREATE_CHARACTER_NAME_ALREADY;
			Send(session, response);
			return;
		}

		auto character = db::Character::Create(*db_conn_, account_id, character_name, (db::ClassType)class_type);
		if (!character)
		{
			// 생성 된게 없다.
			CreateCharacterFailedT response;
			response.error_code = ErrorCode_CREATE_CHARACTER_CANNOT_CREATE;
			Send(session, response);
			return;
		}

		// 초기 능력치 셋팅
		auto lv1_attribute = CharacterAttributeData::GetInstance().Get((db::ClassType)class_type, 1);
		character->SetAttribute(*lv1_attribute);
		character->Update(*db_conn_);
		
		BOOST_LOG_TRIVIAL(info) << "Create Character success : " << character->name;

		auto char_simple = std::make_unique<CharacterSimpleT>();
		char_simple->id = character->id;
		char_simple->name = character->name;
		char_simple->class_type = (int)character->class_type;
		char_simple->level = character->level;

		CreateCharacterSuccessT response;
		response.character = std::move(char_simple);
		
		Send(session, response);
	}

	// Character List ================================================================================================================
	void GameServer::OnCharacterList(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestCreateCharacter*>(net_message->message());
		
		// 로그인 체크
		int account_id = AccountManager::GetInstance().FindAccount(session);
		if (account_id == 0)
		{
			CharacterListFailedT response;
			response.error_code = ErrorCode_INVALID_SESSION;
			Send(session, response);
			return;
		}

		auto char_list = db::Character::Fetch(*db_conn_, account_id);
		
		CharacterListT response;
		for (auto& character : char_list)
		{
			auto char_simple = std::make_unique<CharacterSimpleT>();
			char_simple->id = character->id;
			char_simple->name = character->name;
			char_simple->class_type = (int)character->class_type;
			char_simple->level = character->level;
			response.character_list.emplace_back(std::move(char_simple));
		}

		Send(session, response);
	}

	// Delete Character ==========================================================================================================
	void GameServer::OnDeleteCharacter(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestDeleteCharacter*>(net_message->message());

		// 로그인 체크
		int account_id = AccountManager::GetInstance().FindAccount(session);
		if (account_id == 0)
		{
			DeleteCharacterFailedT response;
			response.error_code = ErrorCode_INVALID_SESSION;
			Send(session, response);
			return;
		}

		const int character_id = msg->character_id();
		auto character = db::Character::Fetch(*db_conn_, character_id, account_id);
		if (!character)
		{
			DeleteCharacterFailedT response;
			response.error_code = ErrorCode_DELETE_CHARACTER_NOT_EXIST;
			Send(session, response);
			return;
		}

		// 삭제
		character->Delete(*db_conn_);

		BOOST_LOG_TRIVIAL(info) << "Delete Character success. Character : " << character->name;

		DeleteCharacterSuccessT response;
		response.character_id = character_id;
		Send(session, response);
	}

	// Enter Game
	void GameServer::OnEnterGame(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestEnterGame*>(net_message->message());
		
		// 로그인 체크
		int account_id = AccountManager::GetInstance().FindAccount(session);
		if (account_id == 0)
		{
			EnterGameFailedT response;
			response.error_code = ErrorCode_INVALID_SESSION;
			Send(session, response);
			return;
		}
	}

	void GameServer::RegisterHandlers()
	{
		AccountManager::GetInstance().RegisterLogoutHandler([this](int account_id, auto& session) {
			BOOST_LOG_TRIVIAL(info) << "Logout. id : " << account_id;
		});

		message_handlers_.insert(make_pair(MessageT_RequestLogin, [this](auto& session, auto* msg) { OnLogin(session, msg); }));
		message_handlers_.insert(make_pair(MessageT_RequestJoin, [this](auto& session, auto* msg) { OnJoin(session, msg); }));
		message_handlers_.insert(make_pair(MessageT_RequestCreateCharacter, [this](auto& session, auto* msg) { OnCreateCharacter(session, msg); }));
		message_handlers_.insert(make_pair(MessageT_RequestCharacterList, [this](auto& session, auto* msg) { OnCharacterList(session, msg); }));
		message_handlers_.insert(make_pair(MessageT_RequestDeleteCharacter, [this](auto& session, auto* msg) {OnDeleteCharacter(session, msg); }));
		message_handlers_.insert(make_pair(MessageT_RequestEnterGame, [this](auto& session, auto* msg) { OnEnterGame(session, msg); }));

		/*message_handlers_.insert(make_pair(MessageT_RequestLogin, std::bind(&GameServer::OnLogin, this, placeholders::_1, placeholders::_2)));
		message_handlers_.insert(make_pair(MessageT_RequestJoin, std::bind(&GameServer::OnJoin, this, placeholders::_1, placeholders::_2)));
		message_handlers_.insert(make_pair(MessageT_RequestCreateCharacter, std::bind(&GameServer::OnCreateCharacter, this, placeholders::_1, placeholders::_2)));
		message_handlers_.insert(make_pair(MessageT_RequestCharacterList, std::bind(&GameServer::OnCharacterList, this, std::placeholders::_1, std::placeholders::_2)));
		message_handlers_.insert(make_pair(MessageT_RequestDeleteCharacter, std::bind(&GameServer::OnDeleteCharacter, this, std::placeholders::_1, std::placeholders::_2)));
		message_handlers_.insert(make_pair(MessageT_RequestEnterGame, std::bind(&GameServer::OnEnterGame, this, std::placeholders::_1, std::placeholders::_2)));*/
	}
}