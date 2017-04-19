#include "stdafx.h"
#include "GameServer.h"
#include "ServerConfig.h"
#include "GameMessageHelper.h"
#include "GameUser.h"

namespace mmog {

	using namespace flatbuffers;
	using namespace protocol;
	using namespace helper;

	void GameServer::Run()
	{
		ServerConfig& server_config = ServerConfig::GetInstance();

		// io_service pool 생성
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
			const NetMessage* net_message = GetNetMessage(buf);
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
				session_closed_handler_(session, reason);
		});

		// Initialize DB
		db_ = std::make_shared<MySQLPool>(
			server_config.db_host,
			server_config.db_user,
			server_config.db_password,
			server_config.db_schema,
			server_config.db_connection_pool);

		// 네트워크 이벤트 핸들러들 등록
		RegisterHandlers();

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

	inline const Ptr<GameUser> GameServer::GetGameUser(const SessionID & session_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		auto iter = user_map_.find(session_id);
		if (iter == user_map_.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	// User UUID로 찾는다.
	inline const Ptr<GameUser> GameServer::GetGameUserByUserID(const uuid& user_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto iter = std::find_if(user_map_.begin(), user_map_.end(), [&user_id](const std::pair<uuid, Ptr<GameUser>>& pair)
			{
				return user_id == pair.second->GetUUID();
			});
		if (iter == user_map_.end())
			return nullptr;

		return iter->second;
	}

	// Account ID로 찾는다
	inline const Ptr<GameUser> GameServer::GetGameUserByAccountID(int account_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto iter = std::find_if(user_map_.begin(), user_map_.end(), [account_id](const std::pair<uuid, Ptr<GameUser>>& pair)
			{
				auto info = pair.second->GetAccountInfo();
				return info.id == account_id;
			});
		if (iter == user_map_.end())
			return nullptr;

		return iter->second;
	}

	inline void GameServer::AddGameUser(SessionID session_id, Ptr<GameUser> user)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		user_map_.insert(std::make_pair(session_id, user));
	}

	inline void GameServer::RemoveGameUser(const SessionID & session_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		user_map_.erase(session_id);
	}

	// 이벤트 핸들러들을 등록한다 ======================================================================================================
	void GameServer::RegisterHandlers()
	{
		this->RegisterSessionOpenHandler([this](const Ptr<Session>& session) { OnSessionOpen(session); });
		this->RegisterSessionCloseHandler([this](const Ptr<Session>& session, CloseReason reason) { OnSessionClose(session, reason); });

		this->RegisterMessageHandler(MessageT_RequestJoin, std::bind(&GameServer::OnRequestJoin, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_RequestLogin, std::bind(&GameServer::OnRequestLogin, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_RequestCreateCharacter, std::bind(&GameServer::OnRequestCreateCharacter, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_RequestCharacterList, std::bind(&GameServer::OnRequestCharacterList, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_RequestDeleteCharacter, std::bind(&GameServer::OnRequestDeleteCharacter, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_RequestEnterGame, std::bind(&GameServer::OnRequestEnterGame, this, std::placeholders::_1, std::placeholders::_2));
	}

	void GameServer::OnSessionOpen(const Ptr<Session>& session)
	{

	}

	void GameServer::OnSessionClose(const Ptr<Session>& session, CloseReason reason)
	{
		// 로그인된 GameUserSession 을 찾는다.
		auto user = GetGameUser(session->GetID());
		if (!user)
			return;

		auto info = user->GetAccountInfo();
		BOOST_LOG_TRIVIAL(info) << "Logout " << info.acc_name << " Session ID : " << session->GetID();
		if (CloseReason::Disconnected == reason)
		{
			user->OnDisconnected();
		}

		// 삭제
		RemoveGameUser(session->GetID());
	}

	// Join ================================================================================================================
	void GameServer::OnRequestJoin(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestJoin*>(net_message->message());

		FlatBufferBuilder builder;

		try
		{
			const char* acc_name = msg->acc_name()->c_str();
			// 문자열 검사
			std::regex pattern(R"([^A-Za-z0-9_]+)");
			std::cmatch m;
			if (std::regex_search(acc_name, m, pattern))
			{
				auto response = CreateJoinFailed(builder, ErrorCode_INVALID_STRING);
				Send(session, builder, response);
				return;
			}

			std::stringstream ss;
			ss << "SELECT acc_name FROM account_tb "
				<< "WHERE acc_name='" << msg->acc_name()->c_str() << "'";

			auto result_set = db_->Excute(ss.str());
			// 이미 있는 계정명. 실패
			if (result_set->rowsCount() > 0)
			{
				auto response = CreateJoinFailed(builder, ErrorCode_JOIN_ACC_NAME_ALREADY);
				Send(session, builder, response);
				return;
			}

			ss.str(""); // 비우기
			ss << "INSERT INTO account_tb (acc_name, password) "
				<< "VALUES ('" << msg->acc_name()->c_str() << "','" << msg->password()->c_str() << "')";

			db_->Excute(ss.str());

			BOOST_LOG_TRIVIAL(info) << "Join success " << msg->acc_name()->c_str();
			// 성공
			auto response = CreateJoinSuccess(builder);
			Send(session, builder, response);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			auto response = CreateJoinFailed(builder, ErrorCode_FATAL_ERROR);
			Send(session, builder, response);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
			
			auto response = CreateJoinFailed(builder, ErrorCode_FATAL_ERROR);
			Send(session, builder, response);
		}
	}

	// Login ================================================================================================================
	void GameServer::OnRequestLogin(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestLogin*>(net_message->message());

		FlatBufferBuilder builder;

		try
		{
			// 세션이 이미 로그인 되있는지 찾는다.
			auto user = GetGameUser(session->GetID());
			if (user)
			{
				// 이미 로그인 되있으면 성공으로 친다.
				auto response = CreateLoginSuccess(builder,
					builder.CreateString(boost::uuids::to_string(session->GetID())));
				Send(session, builder, response);
				return;
			}

			string acc_name = msg->acc_name()->str();
			string password = msg->password()->str();

			std::stringstream ss;
			ss << "SELECT id, acc_name, password FROM account_tb "
				<< "WHERE acc_name='" << acc_name << "' AND password='" << password << "'";

			auto result_set = db_->Excute(ss.str());

			// 결과가 없다. 실패
			if (!result_set->next())
			{
				auto response = CreateLoginFailed(builder,
					ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
				Send(session, builder, response);
				return;
			}

			std::string password2 = result_set->getString("password").c_str();
			// 패스워드가 다르다. 실패
			if (password != password2)
			{
				auto response = CreateLoginFailed(builder,
					ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
				Send(session, builder, response);
				return;
			}

			int acc_id = result_set->getInt("id");
			// 다른 유저 세션에서 이미 로그인된 계정인지 검사.
			auto user2 = GetGameUserByAccountID(acc_id);
			if (user2)
			{
				// 다른 유저 세션 연결을 끊는다.
				user2->Close();
				// 로그인된 유저 목록에서 제거
				RemoveGameUser(user2->GetUUID());
			}

			// 로그인 성공
			// GameUserSession 객체를 만들고 맵에 추가
			AccountInfo acc_info;
			acc_info.id = acc_id;
			acc_info.acc_name = result_set->getString("acc_name").c_str();
			uuid session_id = session->GetID();
			auto game_user = std::make_shared<GameUser>(this, session, acc_info);
			AddGameUser(session_id, game_user);
			BOOST_LOG_TRIVIAL(info) << "Login success : " << acc_info.acc_name;
			// 성공
			auto response = CreateLoginSuccess(builder,
				builder.CreateString(boost::uuids::to_string(session->GetID())));
			Send(session, builder, response);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			auto response = CreateLoginFailed(builder,
				ErrorCode_FATAL_ERROR);
			Send(session, builder, response);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();

			auto response = CreateLoginFailed(builder,
				ErrorCode_FATAL_ERROR);
			Send(session, builder, response);
		}
	}

	// Create Character ================================================================================================================
	void GameServer::OnRequestCreateCharacter(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestCreateCharacter*>(net_message->message());

		try
		{
			auto user = GetGameUser(session->GetID());
			if (!user)
			{
				CreateCharacterFailedT response;
				response.error_code = ErrorCode_INVALID_SESSION;
				Send(session, response);
				return;
			}

			const char* Character_name = msg->name()->c_str();
			int class_type = msg->class_type();

			// 문자열 검사
			std::regex pattern(R"([^A-Za-z0-9_]+)");
			std::cmatch m;
			if (std::regex_search(Character_name, m, pattern))
			{
				CreateCharacterFailedT response;
				response.error_code = ErrorCode_INVALID_STRING;
				Send(session, response);
				return;
			}

			std::stringstream ss;
			// 이름 중복 쿼리
			ss << "SELECT name FROM user_Character_tb "
				<< "WHERE name='" << Character_name << "'";

			auto result_set = db_->Excute(ss.str());
			// 이미 있는 이름. 실패
			if (result_set->rowsCount() > 0)
			{
				CreateCharacterFailedT response;
				response.error_code = ErrorCode_CREATE_CHARACTER_NAME_ALREADY;
				Send(session, response);
				return;
			}

			auto acc_info = user->GetAccountInfo();

			ss.str(""); // 비우기
			// Insert
			ss << "INSERT INTO user_Character_tb (acc_id, name, class_type) "
				<< "VALUES (" << acc_info.id << ",'" << Character_name << "','" << class_type << "')";
			db_->Excute(ss.str());

			ss.str(""); // 비우기
			// Insert 후 생성된 row 조회 (다른 방법이 있을거다?)
			ss << "SELECT id, name, class_type, level FROM user_Character_tb "
				<< "WHERE acc_id=" << acc_info.id << " and name='" << Character_name << "'";

			result_set = db_->Excute(ss.str());
			if (!result_set->next())
			{
				// 생성 된게 없으면 실패
				CreateCharacterFailedT response;
				response.error_code = ErrorCode_CREATE_CHARACTER_CANNOT_CREATE;
				Send(session, response);
				return;
			}

			
			int id = result_set->getInt(1);
			int level = result_set->getInt(4);
			
			auto character = std::make_unique<CharacterSimpleT>();
			character->id = id;
			character->name = Character_name;
			character->class_type = class_type;
			character->level = level;

			CreateCharacterSuccessT response;
			response.character = std::move(character);
			
			// 생성 성공
			BOOST_LOG_TRIVIAL(info) << "Create Character success : " << Character_name;
			Send(session, response);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			CreateCharacterFailedT response;
			response.error_code = ErrorCode_FATAL_ERROR;
			Send(session, response);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();

			CreateCharacterFailedT response;
			response.error_code = ErrorCode_FATAL_ERROR;
			Send(session, response);
		}
	}

	// Character List ================================================================================================================
	void GameServer::OnRequestCharacterList(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestCreateCharacter*>(net_message->message());
		
		try
		{
			auto user = GetGameUser(session->GetID());
			if (!user)
			{
				CharacterListFailedT response;
				response.error_code = ErrorCode_INVALID_SESSION;
				Send(session, response);
				return;
			}

			std::stringstream ss;
			ss << "SELECT id, name, class_type, level FROM user_Character_tb "
				<< "WHERE acc_id=" << user->GetAccountInfo().id << " AND del_type='F'";

			CharacterListT response;

			auto result_set = db_->Excute(ss.str());
			while (result_set->next())
			{
				auto Character = std::make_unique<CharacterSimpleT>();
				Character->id         = result_set->getInt("id");
				Character->name       = result_set->getString("name").c_str();
				Character->class_type = result_set->getInt("class_type");
				Character->level      = result_set->getInt("level");
				response.character_list.emplace_back(std::move(Character));
			}

			Send(session, response);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			CharacterListFailedT response;
			response.error_code = ErrorCode_FATAL_ERROR;
			Send(session, response);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();

			CharacterListFailedT response;
			response.error_code = ErrorCode_FATAL_ERROR;
			Send(session, response);
		}
	}

	// Delete Character ==========================================================================================================
	void GameServer::OnRequestDeleteCharacter(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestDeleteCharacter*>(net_message->message());

		try
		{
			auto user = GetGameUser(session->GetID());
			if (!user)
			{
				DeleteCharacterFailedT response;
				response.error_code = ErrorCode_INVALID_SESSION;
				Send(session, response);
				return;
			}

			const int Character_id = msg->character_id();
			auto acc_info = user->GetAccountInfo();

			std::stringstream ss;
			// del_type 을 'T' 로 업데이트. 실제로 지우지는 않는다.
			ss << "UPDATE user_Character_tb SET "
				<< "del_type='T' "
				<< "WHERE id=" << Character_id << " AND acc_id=" << acc_info.id;

			db_->Excute(ss.str());

			// 성공
			DeleteCharacterSuccessT response;
			response.character_id = Character_id;
			BOOST_LOG_TRIVIAL(info) << "Delete Character success. Character Id:" << Character_id;
			Send(session, response);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			DeleteCharacterFailedT response;
			response.error_code = ErrorCode_FATAL_ERROR;
			Send(session, response);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();

			DeleteCharacterFailedT response;
			response.error_code = ErrorCode_FATAL_ERROR;
			Send(session, response);
		}
	}

	// Enter Game
	void GameServer::OnRequestEnterGame(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestEnterGame*>(net_message->message());

		try
		{
			auto user = GetGameUser(session->GetID());
			if (!user)
			{
				EnterGameFailedT response;
				response.error_code = ErrorCode_INVALID_SESSION;
				Send(session, response);
				return;
			}

			user->EnterGame(msg->character_id());
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			EnterGameFailedT response;
			response.error_code = ErrorCode_FATAL_ERROR;
			Send(session, response);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
			EnterGameFailedT response;
			response.error_code = ErrorCode_FATAL_ERROR;
			Send(session, response);
		}
	}


}