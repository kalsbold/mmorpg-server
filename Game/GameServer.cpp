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

		// io_service pool ����
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
			// flatbuffer �޽����� ��ø��������
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
				// �޽��� �ڵ鷯�� ����
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

		// ��Ʈ��ũ �̺�Ʈ �ڵ鷯�� ���
		RegisterHandlers();

		// NetServer �� ���۽�Ų��.
		std::string bind_address = server_config.bind_address;
		uint16_t bind_port = server_config.bind_port;
		net_server_->Start(bind_address, bind_port);

		BOOST_LOG_TRIVIAL(info) << "Run Game Server";

		// ����ɶ� ���� ���
		ios_pool_->Wait();
	}

	void GameServer::Stop()
	{
		net_server_->Stop();

		// ���� �۾�.


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

	// User UUID�� ã�´�.
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

	// Account ID�� ã�´�
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

	// �̺�Ʈ �ڵ鷯���� ����Ѵ� ======================================================================================================
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
		// �α��ε� GameUserSession �� ã�´�.
		auto user = GetGameUser(session->GetID());
		if (!user)
			return;

		auto info = user->GetAccountInfo();
		BOOST_LOG_TRIVIAL(info) << "Logout " << info.acc_name << " Session ID : " << session->GetID();
		if (CloseReason::Disconnected == reason)
		{
			user->OnDisconnected();
		}

		// ����
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
			// ���ڿ� �˻�
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
			// �̹� �ִ� ������. ����
			if (result_set->rowsCount() > 0)
			{
				auto response = CreateJoinFailed(builder, ErrorCode_JOIN_ACC_NAME_ALREADY);
				Send(session, builder, response);
				return;
			}

			ss.str(""); // ����
			ss << "INSERT INTO account_tb (acc_name, password) "
				<< "VALUES ('" << msg->acc_name()->c_str() << "','" << msg->password()->c_str() << "')";

			db_->Excute(ss.str());

			BOOST_LOG_TRIVIAL(info) << "Join success " << msg->acc_name()->c_str();
			// ����
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
			// ������ �̹� �α��� ���ִ��� ã�´�.
			auto user = GetGameUser(session->GetID());
			if (user)
			{
				// �̹� �α��� �������� �������� ģ��.
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

			// ����� ����. ����
			if (!result_set->next())
			{
				auto response = CreateLoginFailed(builder,
					ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
				Send(session, builder, response);
				return;
			}

			std::string password2 = result_set->getString("password").c_str();
			// �н����尡 �ٸ���. ����
			if (password != password2)
			{
				auto response = CreateLoginFailed(builder,
					ErrorCode_LOGIN_INCORRECT_ACC_NAME_OR_PASSWORD);
				Send(session, builder, response);
				return;
			}

			int acc_id = result_set->getInt("id");
			// �ٸ� ���� ���ǿ��� �̹� �α��ε� �������� �˻�.
			auto user2 = GetGameUserByAccountID(acc_id);
			if (user2)
			{
				// �ٸ� ���� ���� ������ ���´�.
				user2->Close();
				// �α��ε� ���� ��Ͽ��� ����
				RemoveGameUser(user2->GetUUID());
			}

			// �α��� ����
			// GameUserSession ��ü�� ����� �ʿ� �߰�
			AccountInfo acc_info;
			acc_info.id = acc_id;
			acc_info.acc_name = result_set->getString("acc_name").c_str();
			uuid session_id = session->GetID();
			auto game_user = std::make_shared<GameUser>(this, session, acc_info);
			AddGameUser(session_id, game_user);
			BOOST_LOG_TRIVIAL(info) << "Login success : " << acc_info.acc_name;
			// ����
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

			// ���ڿ� �˻�
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
			// �̸� �ߺ� ����
			ss << "SELECT name FROM user_Character_tb "
				<< "WHERE name='" << Character_name << "'";

			auto result_set = db_->Excute(ss.str());
			// �̹� �ִ� �̸�. ����
			if (result_set->rowsCount() > 0)
			{
				CreateCharacterFailedT response;
				response.error_code = ErrorCode_CREATE_CHARACTER_NAME_ALREADY;
				Send(session, response);
				return;
			}

			auto acc_info = user->GetAccountInfo();

			ss.str(""); // ����
			// Insert
			ss << "INSERT INTO user_Character_tb (acc_id, name, class_type) "
				<< "VALUES (" << acc_info.id << ",'" << Character_name << "','" << class_type << "')";
			db_->Excute(ss.str());

			ss.str(""); // ����
			// Insert �� ������ row ��ȸ (�ٸ� ����� �����Ŵ�?)
			ss << "SELECT id, name, class_type, level FROM user_Character_tb "
				<< "WHERE acc_id=" << acc_info.id << " and name='" << Character_name << "'";

			result_set = db_->Excute(ss.str());
			if (!result_set->next())
			{
				// ���� �Ȱ� ������ ����
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
			
			// ���� ����
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
			// del_type �� 'T' �� ������Ʈ. ������ �������� �ʴ´�.
			ss << "UPDATE user_Character_tb SET "
				<< "del_type='T' "
				<< "WHERE id=" << Character_id << " AND acc_id=" << acc_info.id;

			db_->Excute(ss.str());

			// ����
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