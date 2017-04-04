#include "stdafx.h"
#include "GameServer.h"
#include "ServerConfig.h"
#include "GameMessageHelper.h"
#include "GameUser.h"

namespace mmog {

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

	inline const Ptr<GameUser> GameServer::FindGameUser(const uuid & session_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		auto iter = user_map_.find(session_id);
		if (iter == user_map_.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	// Account ID로 찾는다
	inline const Ptr<GameUser> GameServer::FindGameUser(int account_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);

		auto iter = std::find_if(user_map_.begin(), user_map_.end(), [account_id](const std::pair<uuid, Ptr<GameUser>>& pair)
		{
			auto info = pair.second->GetAccountInfo();
			return info.id == account_id;
		});
		if (iter == user_map_.end())
		{
			return nullptr;
		}

		return iter->second;
	}

	inline void GameServer::AddGameUser(uuid session_id, Ptr<GameUser> user)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		user_map_.insert(std::make_pair(session_id, user));
	}

	inline void GameServer::RemoveGameUser(const uuid & session_id)
	{
		std::lock_guard<std::mutex> lock_guard(mutex_);
		user_map_.erase(session_id);
	}

	// Handlers ======================================================================================================
	void GameServer::InitializeHandlers()
	{
		this->RegisterSessionOpenHandler([this](const Ptr<Session>& session) { OnSessionOpen(session); });
		this->RegisterSessionCloseHandler([this](const Ptr<Session>& session, CloseReason reason) { OnSessionClose(session, reason); });

		this->RegisterMessageHandler(MessageT_JoinRequest, std::bind(&GameServer::OnJoinRequest, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_LoginRequest, std::bind(&GameServer::OnLoginRequest, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_CreateHeroRequest, std::bind(&GameServer::OnCreateHeroRequest, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_HeroListRequest, std::bind(&GameServer::OnHeroListRequest, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_DeleteHeroRequest, std::bind(&GameServer::OnDeleteHeroRequest, this, std::placeholders::_1, std::placeholders::_2));
		this->RegisterMessageHandler(MessageT_EnterGameRequest, std::bind(&GameServer::OnEnterGameRequest, this, std::placeholders::_1, std::placeholders::_2));
	}

	void GameServer::OnSessionOpen(const Ptr<Session>& session)
	{

	}

	void GameServer::OnSessionClose(const Ptr<Session>& session, CloseReason reason)
	{
		// 로그인된 GameUser 을 찾는다.
		auto user = FindGameUser(session->ID());
		if (!user)
			return;

		auto info = user->GetAccountInfo();
		BOOST_LOG_TRIVIAL(info) << "Logout " << info.acc_name << " Session ID : " << session->ID();
		if (CloseReason::Disconnected == reason)
		{
			user->OnDisconnected();
		}

		// 삭제
		RemoveGameUser(session->ID());
	}

	// Join ================================================================================================================
	void GameServer::OnJoinRequest(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const JoinRequest*>(net_message->message());

		FlatBufferBuilder builder;

		try
		{
			const char* acc_name = msg->acc_name()->c_str();
			// 문자열 검사
			std::regex pattern(R"([^A-Za-z0-9_]+)");
			std::cmatch m;
			if (std::regex_search(acc_name, m, pattern))
			{
				auto reply = CreateJoinFailedReply(builder, ErrorCode_INVALID_STRING);
				Send(session, builder, reply);
				return;
			}

			std::stringstream ss;
			ss << "SELECT acc_name FROM account_tb "
				<< "WHERE acc_name='" << msg->acc_name()->c_str() << "'";

			auto result_set = db_->Excute(ss.str());
			// 이미 있는 계정명. 실패
			if (result_set->rowsCount() > 0)
			{
				auto reply = CreateJoinFailedReply(builder, ErrorCode_JOIN_ACC_NAME_ALREADY);
				Send(session, builder, reply);
				return;
			}

			ss.str(""); // 비우기
			ss << "INSERT INTO account_tb (acc_name, password) "
				<< "VALUES ('" << msg->acc_name()->c_str() << "','" << msg->password()->c_str() << "')";

			db_->Excute(ss.str());

			BOOST_LOG_TRIVIAL(info) << "Join success " << msg->acc_name()->c_str();
			// 성공
			auto reply = CreateJoinSuccessReply(builder);
			Send(session, builder, reply);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			auto reply = CreateJoinFailedReply(builder, ErrorCode_FATAL_ERROR);
			Send(session, builder, reply);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
			
			auto reply = CreateJoinFailedReply(builder, ErrorCode_FATAL_ERROR);
			Send(session, builder, reply);
		}
	}

	// Login ================================================================================================================
	void GameServer::OnLoginRequest(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const LoginRequest*>(net_message->message());

		FlatBufferBuilder builder;

		try
		{
			// 세션이 이미 로그인 되있는지 찾는다.
			auto user = FindGameUser(session->ID());
			if (user)
			{
				// 이미 로그인 되있으면 성공으로 친다.
				auto reply = CreateLoginSuccessReply(builder,
					builder.CreateString(boost::uuids::to_string(session->ID())));
				Send(session, builder, reply);
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
				auto reply = CreateLoginFailedReply(builder,
					ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
				Send(session, builder, reply);
				return;
			}

			std::string password2 = result_set->getString("password").c_str();
			// 패스워드가 다르다. 실패
			if (password != password2)
			{
				auto reply = CreateLoginFailedReply(builder,
					ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
				Send(session, builder, reply);
				return;
			}

			int acc_id = result_set->getInt("id");
			// 다른 유저 세션에서 이미 로그인된 계정인지 검사.
			auto user2 = FindGameUser(acc_id);
			if (user2)
			{
				// 다른 유저 세션 연결을 끊는다.
				user2->Close();
				// 로그인된 유저 목록에서 제거
				RemoveGameUser(user2->GetSessionID());
			}

			// 로그인 성공
			// GameUser 객체를 만들고 맵에 추가
			AccountInfo acc_info;
			acc_info.id = acc_id;
			acc_info.acc_name = result_set->getString("acc_name").c_str();
			uuid session_id = session->ID();
			auto game_user = std::make_shared<GameUser>(this, session, acc_info);
			AddGameUser(session_id, game_user);
			BOOST_LOG_TRIVIAL(info) << "Login success : " << acc_info.acc_name;
			// 성공
			auto reply = CreateLoginSuccessReply(builder,
				builder.CreateString(boost::uuids::to_string(session->ID())));
			Send(session, builder, reply);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			auto reply = CreateLoginFailedReply(builder,
				ErrorCode_FATAL_ERROR);
			Send(session, builder, reply);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();

			auto reply = CreateLoginFailedReply(builder,
				ErrorCode_FATAL_ERROR);
			Send(session, builder, reply);
		}
	}

	// Create Hero ================================================================================================================
	void GameServer::OnCreateHeroRequest(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const CreateHeroRequest*>(net_message->message());

		try
		{
			auto user = FindGameUser(session->ID());
			if (!user)
			{
				CreateHeroFailedReplyT reply;
				reply.error_code = ErrorCode_INVALID_SESSION;
				Send(session, reply);
				return;
			}

			const char* hero_name = msg->name()->c_str();
			int class_type = msg->class_type();

			// 문자열 검사
			std::regex pattern(R"([^A-Za-z0-9_]+)");
			std::cmatch m;
			if (std::regex_search(hero_name, m, pattern))
			{
				CreateHeroFailedReplyT reply;
				reply.error_code = ErrorCode_INVALID_STRING;
				Send(session, reply);
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
				CreateHeroFailedReplyT reply;
				reply.error_code = ErrorCode_CREATE_HERO_NAME_ALREADY;
				Send(session, reply);
				return;
			}

			auto acc_info = user->GetAccountInfo();

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
				CreateHeroFailedReplyT reply;
				reply.error_code = ErrorCode_CREATE_HERO_CANNOT_CREATE;
				Send(session, reply);
				return;
			}

			
			int id = result_set->getInt(1);
			int level = result_set->getInt(4);
			
			auto hero = std::make_unique<HeroSimpleT>();
			hero->id = id;
			hero->name = hero_name;
			hero->class_type = class_type;
			hero->level = level;

			CreateHeroSuccessReplyT reply;
			reply.hero = std::move(hero);
			
			// 생성 성공
			BOOST_LOG_TRIVIAL(info) << "Create Hero success : " << hero_name;
			Send(session, reply);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			CreateHeroFailedReplyT reply;
			reply.error_code = ErrorCode_FATAL_ERROR;
			Send(session, reply);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();

			CreateHeroFailedReplyT reply;
			reply.error_code = ErrorCode_FATAL_ERROR;
			Send(session, reply);
		}
	}

	// Hero List ================================================================================================================
	void GameServer::OnHeroListRequest(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const CreateHeroRequest*>(net_message->message());
		
		try
		{
			auto user = FindGameUser(session->ID());
			if (!user)
			{
				HeroListFailedReplyT reply;
				reply.error_code = ErrorCode_INVALID_SESSION;
				Send(session, reply);
				return;
			}

			std::stringstream ss;
			ss << "SELECT id, name, class_type, level FROM user_hero_tb "
				<< "WHERE acc_id=" << user->GetAccountInfo().id << " AND del_type='F'";

			HeroListReplyT reply;

			auto result_set = db_->Excute(ss.str());
			while (result_set->next())
			{
				auto hero = std::make_unique<HeroSimpleT>();
				hero->id         = result_set->getInt("id");
				hero->name       = result_set->getString("name").c_str();
				hero->class_type = result_set->getInt("class_type");
				hero->level      = result_set->getInt("level");
				reply.hero_list.emplace_back(std::move(hero));
			}

			Send(session, reply);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			HeroListFailedReplyT reply;
			reply.error_code = ErrorCode_FATAL_ERROR;
			Send(session, reply);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();

			HeroListFailedReplyT reply;
			reply.error_code = ErrorCode_FATAL_ERROR;
			Send(session, reply);
		}
	}

	// Delete Hero ==========================================================================================================
	void GameServer::OnDeleteHeroRequest(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const DeleteHeroRequest*>(net_message->message());

		try
		{
			auto user = FindGameUser(session->ID());
			if (!user)
			{
				DeleteHeroFailedReplyT reply;
				reply.error_code = ErrorCode_INVALID_SESSION;
				Send(session, reply);
				return;
			}

			const int hero_id = msg->hero_id();
			auto acc_info = user->GetAccountInfo();

			std::stringstream ss;
			// del_type 을 'T' 로 업데이트. 실제로 지우지는 않는다.
			ss << "UPDATE user_hero_tb SET "
				<< "del_type='T' "
				<< "WHERE id=" << hero_id << " AND acc_id=" << acc_info.id;

			db_->Excute(ss.str());

			// 성공
			DeleteHeroSuccessReplyT reply;
			reply.hero_id = hero_id;
			BOOST_LOG_TRIVIAL(info) << "Delete Hero success. Hero Id:" << hero_id;
			Send(session, reply);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			DeleteHeroFailedReplyT reply;
			reply.error_code = ErrorCode_FATAL_ERROR;
			Send(session, reply);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();

			DeleteHeroFailedReplyT reply;
			reply.error_code = ErrorCode_FATAL_ERROR;
			Send(session, reply);
		}
	}

	// Enter Game
	void GameServer::OnEnterGameRequest(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const EnterGameRequest*>(net_message->message());

		try
		{
			auto user = FindGameUser(session->ID());
			if (!user)
			{
				EnterGameFailedReplyT reply;
				reply.error_code = ErrorCode_INVALID_SESSION;
				Send(session, reply);
				return;
			}

			// 플레이어 상태 검사
			if (user->GetState() != GameUser::State::Entry)
			{
				EnterGameFailedReplyT reply;
				reply.error_code = ErrorCode_ENTER_GAME_INVALID_STATE;
				Send(session, reply);
				return;
			}

			const int hero_id = msg->hero_id();
			user->BeginEnterGame(hero_id);
		}
		catch (sql::SQLException& e)
		{
			BOOST_LOG_TRIVIAL(info) << "SQL Exception: " << e.what()
				<< ", (MySQL error code : " << e.getErrorCode()
				<< ", SQLState: " << e.getSQLState() << " )";

			EnterGameFailedReplyT reply;
			reply.error_code = ErrorCode_FATAL_ERROR;
			Send(session, reply);
		}
		catch (std::exception& e)
		{
			BOOST_LOG_TRIVIAL(info) << "Exception: " << e.what();
			EnterGameFailedReplyT reply;
			reply.error_code = ErrorCode_FATAL_ERROR;
			Send(session, reply);
		}
	}


}