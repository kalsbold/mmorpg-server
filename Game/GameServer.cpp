#include "stdafx.h"
#include "GameServer.h"
#include "ServerSettings.h"
#include "RemoteClient.h"
#include "World.h"
#include "DBEntity.h"
#include "StaticCachedData.h"
#include "MessageHelper.h"

namespace ph = std::placeholders;
namespace fb = flatbuffers;
namespace proto = protocol;
namespace db = db_entity;

void GameServer::Run()
{
	ServerSettings& settings = ServerSettings::GetInstance();

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
	net_server_->RegisterMessageHandler(std::bind(&GameServer::HandleMessage, this, ph::_1, ph::_2, ph::_3));
	net_server_->RegisterSessionOpenedHandler(std::bind(&GameServer::HandleSessionOpened, this, ph::_1));
	net_server_->RegisterSessionClosedHandler(std::bind(&GameServer::HandleSessionClosed, this, ph::_1, ph::_2));

	/*net_server_->RegisterMessageHandler([this](auto& arg1, auto* arg2, auto arg3) { HandleMessage(arg1, arg2, arg3); });
	net_server_->RegisterSessionOpenedHandler([this](auto& arg1) { HandleSessionOpened(arg1); });
	net_server_->RegisterSessionClosedHandler([this](auto& arg1, auto& arg2) { HandleSessionClosed(arg1, arg2); });*/

	// Create DB connection Pool
	db_conn_ = MySQLPool::Create(
		settings.db_host,
		settings.db_user,
		settings.db_password,
		settings.db_schema,
		settings.db_connection_pool);

	SetName(settings.name);
	RegisterHandlers();

	// DB 데이터 로드
	MapData::Load();
	CharacterAttributeData::Load();

	// Game World 생성및 시작.
	world_ = std::make_shared<World>(ios_loop_);
	world_->Start();

	// NetServer 를 시작시킨다.
	std::string bind_address = settings.bind_address;
	uint16_t bind_port = settings.bind_port;
	net_server_->Start(bind_address, bind_port);
		
	BOOST_LOG_TRIVIAL(info) << "Run Game Server : " << GetName();
}

void GameServer::Stop()
{
	// 종료 작업.
	net_server_->Stop();
		
	world_->Stop();
		
	ios_loop_->Stop();

	BOOST_LOG_TRIVIAL(info) << "Stop Game Server : " << GetName();
}

// 서버가 종료될 때가지 대기
void GameServer::Wait()
{
	if (ios_loop_ == nullptr)
		return;

	ios_loop_->Wait();
}

const Ptr<RemoteClient> GameServer::GetRemoteClient(int session_id)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);

	auto iter = remote_clients_.find(session_id);
	if (iter == remote_clients_.end())
		return nullptr;

	return iter->second;
}

const Ptr<RemoteClient> GameServer::GetRemoteClientByAccountID(int account_id)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);

	auto iter = std::find_if(remote_clients_.begin(), remote_clients_.end(), [account_id](auto& var) {
		return (var.second->GetAccount()->id == account_id);
	});
	if (iter == remote_clients_.end())
		return nullptr;

	return iter->second;
}

void GameServer::AddRemoteClient(int session_id, Ptr<RemoteClient> remote_client)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);

	remote_clients_.emplace(session_id, remote_client);
	BOOST_LOG_TRIVIAL(info) << "RemoteClient count:" << remote_clients_.size();
}

void GameServer::RemoveRemoteClient(int session_id)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);

	remote_clients_.erase(session_id);
	BOOST_LOG_TRIVIAL(info) << "RemoteClient count:" << remote_clients_.size();
}

void GameServer::NotifyUnauthedAccess(const Ptr<net::Session>& session)
{
	fb::FlatBufferBuilder builder;
	auto reply = proto::CreateNotifyUnauthedAccess(builder);
	helper::Send(session, builder, reply);
}

void GameServer::HandleMessage(const Ptr<net::Session>& session, const uint8_t* buf, size_t bytes)
{
	// flatbuffer 메시지로 디시리얼라이즈
	const proto::NetMessage* net_message = proto::GetNetMessage(buf);
	if (net_message == nullptr)
	{
		BOOST_LOG_TRIVIAL(info) << "Invalid NetMessage";
		return;
	}

	auto message_type = net_message->message_type();
	auto it = message_handlers_.find(message_type);

	if (it == message_handlers_.end())
	{
		BOOST_LOG_TRIVIAL(info) << "Can not find the message handler. message_type : " << EnumNameMessageT(message_type);
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

void GameServer::HandleSessionOpened(const Ptr<net::Session>& session)
{

}

void GameServer::HandleSessionClosed(const Ptr<net::Session>& session, net::CloseReason reason)
{
	auto remote_client = GetRemoteClient(session->GetID());
	if (!remote_client)
		return;

	// 종료 처리
	remote_client->OnDisconnect();
	BOOST_LOG_TRIVIAL(info) << "Logout : " << remote_client->GetAccount()->acc_name;

	// 목록에서 제거
	RemoveRemoteClient(session->GetID());
}

// Login ================================================================================================================
void GameServer::OnRequestLogin(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = static_cast<const proto::login::RequestLogin*>(net_message->message());

	fb::FlatBufferBuilder builder;

	const std::string acc_name = msg->acc_name()->str();
	const std::string password = msg->password()->str();

	auto db_account = db::Account::Fetch(db_conn_, acc_name);
	// 계정이 없음
	if (!db_account)
	{
		auto reply = proto::login::CreateReplyLoginFailed(builder,
			proto::ErrorCode::LOGIN_INCORRECT_ACC_NAME);
		helper::Send(session, builder, reply);
		return;
	}
	// 비번이 다름
	if (db_account->password != password)
	{
		auto reply = proto::login::CreateReplyLoginFailed(builder,
			proto::ErrorCode::LOGIN_INCORRECT_ACC_PASSWORD);
		helper::Send(session, builder, reply);
		return;
	}
	
	// 이미 로그인 되어 있으면
	if (auto rc = GetRemoteClientByAccountID(db_account->id))
	{
		auto reply = proto::login::CreateReplyLoginFailed(builder, proto::ErrorCode::LOGIN_DUPLICATION);
		helper::Send(session, builder, reply);
		session->Close();

		if (rc->GetSession() != session)
		{
			helper::Send(rc->GetSession(), builder, reply);
			rc->Disconnect();
		}
		return;
	}

	// 새 RemoteClient 생성 및 추가
	auto new_rc = RemoteClient::Create(this, boost::uuids::random_generator()(), session);
	new_rc->SetLogin(db_account);
	AddRemoteClient(session->GetID(), new_rc);

	BOOST_LOG_TRIVIAL(info) << "Login : " << db_account->acc_name;

	auto reply = proto::login::CreateReplyLoginSuccess(builder);
	helper::Send(session, builder, reply);
}

// Join ================================================================================================================
void GameServer::OnRequestJoin(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = static_cast<const proto::login::RequestJoin*>(net_message->message());

	const char* acc_name = msg->acc_name()->c_str();
	const char* password = msg->password()->c_str();
		
	// 문자열 검사
	std::regex pattern(R"([^A-Za-z0-9_]+)");
	std::cmatch m;
	if (std::regex_search(acc_name, m, pattern))
	{
		proto::login::ReplyJoinFailedT reply;
		reply.error_code = proto::ErrorCode::INVALID_STRING;
		helper::Send(session, reply);
		return;
	}

	if (db::Account::Fetch(db_conn_, acc_name))
	{
		proto::login::ReplyJoinFailedT reply;
		reply.error_code = proto::ErrorCode::JOIN_ACC_NAME_ALREADY;
		helper::Send(session, reply);
		return;
	}

	if (!db::Account::Create(db_conn_, acc_name, password))
	{
		proto::login::ReplyJoinFailedT reply;
		reply.error_code = proto::ErrorCode::JOIN_CANNOT_ACC_CREATE;
		helper::Send(session, reply);
		return;
	}

	BOOST_LOG_TRIVIAL(info) << "Join : " << acc_name;

	proto::login::ReplyJoinSuccessT reply;
	helper::Send(session, reply);
}

// Create Character ================================================================================================================
void GameServer::OnRequestCreateCharacter(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = static_cast<const proto::login::RequestCreateCharacter*>(net_message->message());
		
	// 로그인 체크
	auto remote_client = GetRemoteClient(session->GetID());
	if (!remote_client)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	const char* character_name = msg->name()->c_str();
	const ClassType class_type = (ClassType)msg->class_type();
	// 문자열 검사
	std::regex pattern(R"([^A-Za-z0-9_]+)");
	std::cmatch m;
	if (std::regex_search(character_name, m, pattern))
	{
		proto::login::ReplyCreateCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::INVALID_STRING;
		helper::Send(session, reply);
		return;
	}

	if (db::Character::Fetch(db_conn_, character_name))
	{
		// 이미 있는 이름.
		proto::login::ReplyCreateCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::CREATE_CHARACTER_NAME_ALREADY;
		helper::Send(session, reply);
		return;
	}

	// 초기 능력치를 가져온다.
	const int level = 1;
	auto attribute = CharacterAttributeData::GetInstance().Get(class_type, level);
	if (!attribute)
	{
		proto::login::ReplyCreateCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::CREATE_CHARACTER_ATTRIBUTE_NOT_EXIST;
		helper::Send(session, reply);
		return;
	}

	//생성
	auto db_character = db::Character::Create(db_conn_, remote_client->GetAccount()->id, character_name, class_type);
	if (!db_character)
	{
		// 생성 된게 없다.
		proto::login::ReplyCreateCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::CREATE_CHARACTER_CANNOT_CREATE;
		helper::Send(session, reply);
		return;
	}

	// 초기 능력치로 셋.
	db_character->SetAttribute(*attribute);
	db_character->Update();
		
	BOOST_LOG_TRIVIAL(info) << "Create Character : " << db_character->name;

	auto character = std::make_unique<proto::login::CharacterT>();
	character->id = db_character->id;
	character->name = db_character->name;
	character->class_type = (proto::ClassType)db_character->class_type;
	character->level = db_character->level;

	proto::login::ReplyCreateCharacterSuccessT reply;
	reply.character = std::move(character);
	helper::Send(session, reply);
}

// Character List ================================================================================================================
void GameServer::OnRequestCharacterList(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = static_cast<const proto::login::RequestCharacterList*>(net_message->message());
		
	// 로그인 체크
	auto remote_client = GetRemoteClient(session->GetID());
	if (!remote_client)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	auto db_char_list = db::Character::Fetch(db_conn_, remote_client->GetAccount()->id);
		
	proto::login::ReplyCharacterListT reply;
	for (auto& var : db_char_list)
	{
		auto character = std::make_unique<proto::login::CharacterT>();
		character->id = var->id;
		character->name = var->name;
		character->class_type = (proto::ClassType)var->class_type;
		character->level = var->level;
		reply.list.emplace_back(std::move(character));
	}

	helper::Send(session, reply);
}

// Delete Character ==========================================================================================================
void GameServer::OnRequestDeleteCharacter(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = static_cast<const proto::login::RequestDeleteCharacter*>(net_message->message());

	// 로그인 체크
	auto remote_client = GetRemoteClient(session->GetID());
	if (!remote_client)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	const int character_id = msg->character_id();

	auto db_character = db::Character::Fetch(db_conn_, character_id, remote_client->GetAccount()->id);
	if (!db_character)
	{
		proto::login::ReplyDeleteCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::DELETE_CHARACTER_NOT_EXIST;
		helper::Send(session, reply);
		return;
	}

	// 삭제
	db_character->Delete();

	BOOST_LOG_TRIVIAL(info) << "Delete Character : " << db_character->name;

	proto::login::ReplyDeleteCharacterSuccessT reply;
	reply.character_id = character_id;
	helper::Send(session, reply);
}

// Enter World
void GameServer::OnRequestEnterWorld(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = static_cast<const proto::world::RequestEnterWorld*>(net_message->message());
		
	// 로그인 체크
	auto remote_client = GetRemoteClient(session->GetID());
	if (!remote_client)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	/*
	Account acc_info;
	acc_info.id = acc_id;
	acc_info.acc_name = result_set->getString("acc_name").c_str();
	uuid session_id = session->GetID();
	auto game_user = std::make_shared<GamePlayer>(this, session, acc_info);
	AddGamePlayer(session_id, game_user);
	BOOST_LOG_TRIVIAL(info) << "Login success : " << acc_info.acc_name;
	// 성공
	auto reply = CreateLoginSuccess(builder,
	builder.CreateString(boost::uuids::to_string(session->GetID())));
	Send(session, builder, reply);
	*/
}

void GameServer::RegisterHandlers()
{
	message_handlers_.insert(std::make_pair(proto::MessageT::login_RequestLogin, std::bind(&GameServer::OnRequestLogin, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::login_RequestJoin, std::bind(&GameServer::OnRequestJoin, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::login_RequestCreateCharacter, std::bind(&GameServer::OnRequestCreateCharacter, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::login_RequestCharacterList, std::bind(&GameServer::OnRequestCharacterList, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::login_RequestDeleteCharacter, std::bind(&GameServer::OnRequestDeleteCharacter, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::world_RequestEnterWorld, std::bind(&GameServer::OnRequestEnterWorld, this, ph::_1, ph::_2)));
}
