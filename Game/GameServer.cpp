#include "stdafx.h"
#include "GameServer.h"
#include "ServerConfig.h"
#include "GamePlayer.h"
#include "DatabaseEntity.h"
#include "AccountManager.h"
#include "MessageHelper.h"

namespace mmog {

	using namespace std;
	using namespace flatbuffers;
	using namespace protocol;
	using namespace helper;

	void GameServer::Run()
	{
		ServerConfig& server_config = ServerConfig::GetInstance();

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

		// Create DB connection Pool
		db_ = std::make_shared<MySQLPool>(
			server_config.db_host,
			server_config.db_user,
			server_config.db_password,
			server_config.db_schema,
			server_config.db_connection_pool);

		// NetServer �� ���۽�Ų��.
		std::string bind_address = server_config.bind_address;
		uint16_t bind_port = server_config.bind_port;
		net_server_->Start(bind_address, bind_port);

		SetName(server_config.name);
		BOOST_LOG_TRIVIAL(info) << "Run Game Server : " << GetName();

		// ����ɶ� ���� ���
		ios_pool_->Wait();
	}

	void GameServer::Stop()
	{
		net_server_->Stop();

		// ���� �۾�.


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
		// flatbuffer �޽����� ��ø��������
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
			// �޽��� �ڵ鷯�� ����
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
		// �α׾ƿ� ó��
		int id = AccountManager::GetInstance().FindAccount(session);
		AccountManager::GetInstance().SetLogout(id);

		// �������� �÷��̾�
		auto player = GetGamePlayer(session->GetID());
		if (!player)
			return;

		// Ŭ���̾�Ʈ ������ ������ ��� ���� ó��
		if (CloseReason::Disconnected == reason)
		{
			player->OnDisconnected();
		}

		// ����
		RemoveGamePlayer(session->GetID());
	}

	// Login ================================================================================================================
	void GameServer::OnLogin(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestLogin*>(net_message->message());

		FlatBufferBuilder builder;

		string acc_name = msg->acc_name()->str();
		string password = msg->password()->str();

		auto account = Account::Fetch(*db_, acc_name);
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

		// �α��� üũ
		if (!AccountManager::GetInstance().CheckAndSetLogin(account->id, session))
		{
			// �α��� �Ǿ��ִ� ������ ��´�
			auto logged_in_session = AccountManager::GetInstance().FindSession(account->id);
			// ���� ������ �ƴϸ�
			if(logged_in_session != session)
			{
				// ���� ���� �α׾ƿ�
				AccountManager::GetInstance().SetLogout(account->id);
				// ���� �÷��� ���� ó��
				auto player = GetGamePlayer(logged_in_session->GetID());
				player->Disconnect();
				// �� ���� �α���
				AccountManager::GetInstance().CheckAndSetLogin(account->id, session);
			}
		}

		// �α��� ���� response
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
		// ����
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

	

	// Create Character ================================================================================================================
	void GameServer::OnCreateCharacter(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestCreateCharacter*>(net_message->message());

		try
		{
			auto user = GetGamePlayer(session->GetID());
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
	void GameServer::OnCharacterList(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestCreateCharacter*>(net_message->message());
		
		try
		{
			auto user = GetGamePlayer(session->GetID());
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
	void GameServer::OnDeleteCharacter(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestDeleteCharacter*>(net_message->message());

		try
		{
			auto user = GetGamePlayer(session->GetID());
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
	void GameServer::OnEnterGame(const Ptr<Session>& session, const NetMessage * net_message)
	{
		auto msg = static_cast<const RequestEnterGame*>(net_message->message());

		try
		{
			auto user = GetGamePlayer(session->GetID());
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

	void GameServer::RegisterHandlers()
	{
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