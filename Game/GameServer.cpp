#include "stdafx.h"
#include "GameServer.h"
#include "ServerSettings.h"
#include "RemoteClient.h"
#include "Zone.h"
#include "Actor.h"
#include "DBSchema.h"
#include "StaticCachedData.h"
#include "ProtocolHelper.h"

namespace ph = std::placeholders;
namespace fb = flatbuffers;
namespace proto = protocol;
namespace db = db_schema;

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
	MapTable::Load();
	CharacterAttributeTable::Load();

	// Game World 생성및 시작.
	InitWorld();

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
}

void GameServer::RemoveRemoteClient(int session_id)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);

	remote_clients_.erase(session_id);
}

void GameServer::RemoveRemoteClient(const Ptr<RemoteClient>& remote_client)
{
	std::lock_guard<std::mutex> lock_guard(mutex_);

	remote_clients_.erase(remote_client->GetSessionID());
}

void GameServer::ProcessRemoteClientDisconnected(const Ptr<RemoteClient>& rc)
{
	// 목록에서 제거
	RemoveRemoteClient(rc);

	BOOST_LOG_TRIVIAL(info) << "Logout : " << rc->GetAccount()->user_name;

	// Process leave the game world
	strand_->post([this, rc]()
	{
		ExitZone(rc->GetCharacter());
	});
}

void GameServer::InitWorld()
{
	strand_ = std::make_shared<boost::asio::strand>(ios_loop_->GetIoService());

	CreateFieldZones();

	update_timer_ = std::make_shared<timer>(strand_->get_io_service());
	ScheduleNextUpdate();
}

// 필드 존 생성
void GameServer::CreateFieldZones()
{
	auto& map_table = MapTable::GetInstance().GetAll();
	for (auto& map : map_table)
	{
		if (map->type == MapType::FIELD)
		{
			auto zone = std::make_shared<Zone>(*map);
			field_zones_.push_back(zone);
		}
	}
}

// 필드 존 객체를 가져온다.
Ptr<Zone> GameServer::GetFieldZone(int map_id)
{
	auto iter = std::find_if(field_zones_.begin(), field_zones_.end(),
		[map_id](const Ptr<Zone>& zone)
	{
		return zone->map_data_.id == map_id;
	});
	if (iter == field_zones_.end())
		return nullptr;

	return *iter;
}

void GameServer::EnterZone(Ptr<PlayerCharacter> pc, Zone * zone, Vector3 pos)
{
	if (!pc || !zone)
		return;

	// 이미 입장 되있는지 검사.
	auto iter = zone->players_.find(pc->GetUUID());
	if (iter != zone->players_.end())
		return;

	// Zone 객체를 셋
	pc->SetLocationZone(zone);
	pc->SetPosition(pos);

	fb::FlatBufferBuilder fbb;
	// 존 입장 메시지를 보낸다.
	protocol::world::MoveInfo move_info(pc->GetRotation(), cast_as<protocol::Vec3>(pc->GetPosition()), protocol::Vec3(0,0,0));
	auto notify_enter_zone = protocol::world::CreateNotify_EnterZone(fbb, zone->map_data_.id, &move_info);
	auto net_msg = CreateNetMessageHelper(fbb, notify_enter_zone);
	fbb.Finish(net_msg);
	pc->GetRemoteClient()->Send(fbb);

	fbb.Clear();
	// 다른 플레이어에 통지
	auto remote_pc = pc->SerializeAs<protocol::world::RemotePC>(fbb);

	std::vector<fb::Offset<protocol::world::RemotePC>> remote_pc_vector;
	remote_pc_vector.push_back(remote_pc);
	auto notify_appear_actor = protocol::world::CreateNotify_AppearActorDirect(fbb, &remote_pc_vector);

	net_msg = CreateNetMessageHelper(fbb, notify_appear_actor);
	fbb.Finish(net_msg);
	// 여러 명에게 보내기 때문에 공유버퍼 사용.
	auto buffer = Buffer(fbb);

	// 지역의 플레이어에게 등장 메시지를 보낸다.
	// TO DO : 공간 분할(격자, 쿼드트리 등)
	for (auto& var : zone->players_)
	{
		var.second->GetRemoteClient()->Send(buffer);
	}

	fbb.Clear();
	remote_pc_vector.clear();

	// 자신에게 주변 오브젝트 정보를 보낸다.
	// TO DO : 공간 분할(격자, 쿼드트리 등)
	std::vector<fb::Offset<protocol::world::Monster>> monster_vector;

	for (auto& var : zone->players_)
	{
		auto remote_pc = var.second->SerializeAs<protocol::world::RemotePC>(fbb);
		remote_pc_vector.push_back(remote_pc);
	}

	for (auto& var : zone->monsters_)
	{
		auto monster = var.second->Serialize(fbb);
		monster_vector.push_back(monster);
	}

	notify_appear_actor = protocol::world::CreateNotify_AppearActorDirect(fbb, &remote_pc_vector, &monster_vector);
	net_msg = CreateNetMessageHelper(fbb, notify_appear_actor);
	fbb.Finish(net_msg);
	pc->GetRemoteClient()->Send(fbb);

	// Zone 입장 목록에 넣어줌
	zone->players_.emplace(pc->GetUUID(), pc);
}

void GameServer::ExitZone(Ptr<PlayerCharacter> pc)
{
	if (!pc)
		return;

	Zone* zone = pc->GetLocationZone();
	if (!zone)
		return;

	// 목록에서 제거
	size_t num = zone->players_.erase(pc->GetUUID());
	if (num == 0)
		return;

	// 다른 플레이어에 소멸 통지
	fb::FlatBufferBuilder fbb;

	auto uuid = fbb.CreateString(boost::uuids::to_string(pc->GetUUID()));
	std::vector<fb::Offset<fb::String>> uuid_vector;
	uuid_vector.push_back(uuid);
	auto notify = protocol::world::CreateNotify_DisappearActorDirect(fbb, &uuid_vector);

	auto net_message = CreateNetMessageHelper(fbb, notify);
	fbb.Finish(net_message);

	auto buffer = Buffer(fbb);

	// 지역의 플레이어에게 소멸 메시지를 보낸다.
	// TO DO : 공간 분할(격자, 쿼드트리 등)
	for (auto& var : zone->players_)
	{
		var.second->GetRemoteClient()->Send(buffer);
	}

	pc->SetLocationZone(nullptr);
}

void GameServer::ScheduleNextUpdate()
{
	auto start_time = clock::now();
	auto update_time = start_time + timestep;
	update_timer_->expires_at(update_time);
	update_timer_->async_wait(strand_->wrap([this, start_time](auto& error)
	{
		if (error)
			return;

		double delta_time = double_seconds(clock::now() - start_time).count();
		ScheduleNextUpdate();
		DoUpdate(delta_time);
	}));
}

void GameServer::NotifyUnauthedAccess(const Ptr<net::Session>& session)
{
	fb::FlatBufferBuilder fbb;
	auto notify = proto::CreateNotify_UnauthedAccess(fbb);
	auto net_msg = CreateNetMessageHelper(fbb, notify);
	fbb.Finish(net_msg);
	SendFlatBuffer(session, fbb);
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

	remote_client->OnDisconnected();
	ProcessRemoteClientDisconnected(remote_client);
}

// Join ================================================================================================================
void GameServer::OnJoin(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = net_message->message_as<proto::login::Request_Join>();
	if (msg == nullptr) return;

	const char* acc_name = msg->acc_name()->c_str();
	const char* password = msg->password()->c_str();

	// 문자열 검사
	std::regex pattern(R"([^A-Za-z0-9_]+)");
	std::cmatch m;
	if (std::regex_search(acc_name, m, pattern))
	{
		proto::login::Reply_JoinFailedT reply;
		reply.error_code = proto::ErrorCode::INVALID_STRING;
		SendProtocolMessage(session, reply);
		return;
	}

	if (db::Account::Fetch(db_conn_, acc_name))
	{
		proto::login::Reply_JoinFailedT reply;
		reply.error_code =	proto::ErrorCode::JOIN_ACC_NAME_ALREADY;
		SendProtocolMessage(session, reply);
		return;
	}

	if (!db::Account::Create(db_conn_, acc_name, password))
	{
		proto::login::Reply_JoinFailedT reply;
		reply.error_code = proto::ErrorCode::JOIN_CANNOT_ACC_CREATE;
		SendProtocolMessage(session, reply);
		return;
	}

	BOOST_LOG_TRIVIAL(info) << "Join : " << acc_name;

	proto::login::Reply_JoinSuccessT reply;
	SendProtocolMessage(session, reply);
}

// Login ================================================================================================================
void GameServer::OnLogin(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = net_message->message_as<proto::login::Request_Login>();
	if (msg == nullptr) return;

	const std::string acc_name = msg->acc_name()->str();
	const std::string password = msg->password()->str();

	auto db_account = db::Account::Fetch(db_conn_, acc_name);
	// 계정이 없다.
	if (!db_account)
	{
		proto::login::Reply_LoginFailedT reply;
		reply.error_code = proto::ErrorCode::LOGIN_INCORRECT_ACC_NAME;
		SendProtocolMessage(session, reply);
		return;
	}
	// 비번이 다르다.
	if (db_account->password != password)
	{
		proto::login::Reply_LoginFailedT reply;
		reply.error_code = proto::ErrorCode::LOGIN_INCORRECT_ACC_PASSWORD;
		SendProtocolMessage(session, reply);
		return;
	}
	
	// 이미 로그인 되어 있는 계정이면.
	if (auto rc = GetRemoteClientByAccountID(db_account->id))
	{
		proto::login::Reply_LoginFailedT reply;
		reply.error_code = proto::ErrorCode::LOGIN_DUPLICATION;

		// RemoteClient 접속을 끊어준다.
		rc->Send(reply);
		rc->Disconnect();
		// 종료처리를 바로 해준다.
		ProcessRemoteClientDisconnected(rc);

		// 같은 세션이 아니었으면 신규 세션도 접속을 끊어준다.
		if (rc->GetSession() != session)
		{
			SendProtocolMessage(session, reply);
			session->Close();
		}
		return;
	}

	// 로그인 성공. RemoteClient 생성 및 추가.
	auto new_rc = std::make_shared<RemoteClient>(this, session);
	new_rc->SetAuthentication(boost::uuids::random_generator()());
	new_rc->SetAccount(db_account);
	AddRemoteClient(session->GetID(), new_rc);

	BOOST_LOG_TRIVIAL(info) << "Login : " << db_account->user_name;

	proto::login::Reply_LoginSuccessT reply;
	reply.auth_key = boost::uuids::to_string(new_rc->GetAuthKey());
	SendProtocolMessage(session, reply);
}

// Create Character ================================================================================================================
void GameServer::OnCreateCharacter(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = net_message->message_as<proto::login::Request_CreateCharacter>();
	if (msg == nullptr) return;
		
	// 로그인 체크
	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
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
		proto::login::Reply_CreateCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::INVALID_STRING;
		SendProtocolMessage(session, reply);
		return;
	}

	if (db::Character::Fetch(db_conn_, character_name))
	{
		// 이미 있는 이름.
		proto::login::Reply_CreateCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::CREATE_CHARACTER_NAME_ALREADY;
		SendProtocolMessage(session, reply);
		return;
	}

	// 초기 능력치를 가져온다.
	const int level = 1;
	auto attribute = CharacterAttributeTable::GetInstance().Get(class_type, level);
	if (!attribute)
	{
		proto::login::Reply_CreateCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::CREATE_CHARACTER_ATTRIBUTE_NOT_EXIST;
		SendProtocolMessage(session, reply);
		return;
	}

	//생성
	auto db_character = db::Character::Create(db_conn_, rc->GetAccount()->id, character_name, class_type);
	if (!db_character)
	{
		// 생성 된게 없다.
		proto::login::Reply_CreateCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::CREATE_CHARACTER_CANNOT_CREATE;
		SendProtocolMessage(session, reply);
		return;
	}

	// 초기 능력치로 셋.
	db_character->SetAttribute(*attribute);
	db_character->map_id = 1001;	// 시작맵
	db_character->pos = Vector3(100.0f, 0.0f, 100.0f); // 시작 좌표
	db_character->Update();
		
	BOOST_LOG_TRIVIAL(info) << "Create Character : " << db_character->name;

	auto character = std::make_unique<proto::login::CharacterT>();
	character->id = db_character->uid;
	character->name = db_character->name;
	character->class_type = (proto::ClassType)db_character->class_type;
	character->level = db_character->level;

	proto::login::Reply_CreateCharacterSuccessT reply;
	reply.character = std::move(character);
	SendProtocolMessage(session, reply);
}

// Character List ================================================================================================================
void GameServer::OnCharacterList(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = net_message->message_as<proto::login::Request_CharacterList>();
	//auto msg = static_cast<const proto::login::Request_CharacterList*>(net_message->message());
	if (msg == nullptr) return;
		
	// 로그인 체크
	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	auto db_char_list = db::Character::Fetch(db_conn_, rc->GetAccount()->id);
		
	proto::login::Reply_CharacterListT reply;
	for (auto& var : db_char_list)
	{
		auto character = std::make_unique<proto::login::CharacterT>();
		character->id = var->id;
		character->name = var->name;
		character->class_type = (proto::ClassType)var->class_type;
		character->level = var->level;
		reply.list.emplace_back(std::move(character));
	}

	SendProtocolMessage(session, reply);
}

// Delete Character ==========================================================================================================
void GameServer::OnDeleteCharacter(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = net_message->message_as<proto::login::Request_DeleteCharacter>();
	if (msg == nullptr) return;

	// 로그인 체크
	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	const int character_id = msg->character_id();

	auto db_character = db::Character::Fetch(db_conn_, character_id, rc->GetAccount()->id);
	if (!db_character)
	{
		proto::login::Reply_DeleteCharacterFailedT reply;
		reply.error_code = proto::ErrorCode::DELETE_CHARACTER_NOT_EXIST;
		SendProtocolMessage(session, reply);
		return;
	}

	// 삭제
	db_character->Delete();

	BOOST_LOG_TRIVIAL(info) << "Delete Character : " << db_character->name;

	proto::login::Reply_DeleteCharacterSuccessT reply;
	reply.character_id = character_id;
	SendProtocolMessage(session, reply);
}

// Enter World
void GameServer::OnEnterWorld(const Ptr<net::Session>& session, const proto::NetMessage * net_message)
{
	auto msg = net_message->message_as<proto::world::Request_EnterWorld>();
	if (msg == nullptr) return;
		
	// 로그인 체크
	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	const int character_id = msg->character_id();

	// 플레이어 상태 검사
	if (rc->GetState() != RemoteClient::State::Connected)
	{
		protocol::world::Reply_EnterWorldFailedT reply;
		reply.error_code = protocol::ErrorCode::ENTER_WORLD_INVALID_STATE;
		rc->Send(reply);
		return;
	}

	// 캐릭터 로드
	auto db_character = db::Character::Fetch(GetDB(), character_id, rc->GetAccount()->id);
	// 캐릭터 로드 실패
	if (!db_character)
	{
		protocol::world::Reply_EnterWorldFailedT reply;
		reply.error_code = protocol::ErrorCode::ENTER_WORLD_INVALID_CHARACTER;
		rc->Send(reply);
		return;
	}

	// 케릭터 인스턴스 생성
	auto character = std::make_shared<PlayerCharacter>(boost::uuids::random_generator()(), rc.get(), db_character);
	rc->SetCharacter(character);

	// 성공 메시지를 보낸다
	protocol::world::Reply_EnterWorldSuccessT reply;
	rc->Send(reply);
}

// 클라이언트 World 씬 로드 완료
void GameServer::OnEnterWorldNext(const Ptr<net::Session>& session, const protocol::NetMessage * net_message)
{
	auto msg = net_message->message_as<proto::world::Request_EnterWorldNext>();
	if (msg == nullptr) return;

	// 로그인 체크
	auto rc = GetRemoteClient(session->GetID());
	if (!rc)
	{
		NotifyUnauthedAccess(session);
		return;
	}

	// 플레이어 상태 검사
	if (rc->GetState() != RemoteClient::State::Connected)
	{
		protocol::world::Reply_EnterWorldNextFailedT reply;
		reply.error_code = protocol::ErrorCode::ENTER_WORLD_NEXT_INVALID_STATE;
		rc->Send(reply);
		return;
	}

	// 로드된 캐릭터가 없으면 실패.
	auto pc = rc->GetCharacter();
	if (!pc)
	{
		protocol::world::Reply_EnterWorldNextFailedT reply;
		reply.error_code = protocol::ErrorCode::ENTER_WORLD_NEXT_CHARACTER_NOT_LOADED;
		rc->Send(reply);
		return;
	}

	// 입장할 Zone 객체를 얻는다.
	auto field_zone = GetFieldZone(pc->map_id_);
	if (!field_zone)
	{
		protocol::world::Reply_EnterWorldNextFailedT reply;
		reply.error_code = protocol::ErrorCode::ENTER_WORLD_NEXT_CANNOT_FIND_ZONE;
		rc->Send(reply);
		return;
	}

	// 성공 메시지.
	fb::FlatBufferBuilder fbb;
	auto player_character = pc->SerializeAs<protocol::world::PlayerCharacter>(fbb);
	auto reply = protocol::world::CreateReply_EnterWorldNextSuccess(fbb, player_character);
	auto net_msg = CreateNetMessageHelper(fbb, reply);
	fbb.Finish(net_msg);
	rc->Send(fbb);

	// Process enter the game world 
	rc->SetState(RemoteClient::State::WorldEntering);
	strand_->post([this, rc, pc, field_zone]()
	{
		// 게임 존 진입.
		EnterZone(pc, field_zone.get(), pc->GetPosition());
		rc->SetState(RemoteClient::State::WorldEntered);
	});
}

void GameServer::OnMove(const Ptr<net::Session>& session, const protocol::NetMessage * net_message)
{
}

void GameServer::OnAttack(const Ptr<net::Session>& session, const protocol::NetMessage * net_message)
{
}

void GameServer::RegisterHandlers()
{
	message_handlers_.insert(std::make_pair(proto::MessageT::login_Request_Login, std::bind(&GameServer::OnLogin, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::login_Request_Join, std::bind(&GameServer::OnJoin, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::login_Request_CreateCharacter, std::bind(&GameServer::OnCreateCharacter, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::login_Request_CharacterList, std::bind(&GameServer::OnCharacterList, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::login_Request_DeleteCharacter, std::bind(&GameServer::OnDeleteCharacter, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::world_Request_EnterWorld, std::bind(&GameServer::OnEnterWorld, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::world_Request_EnterWorldNext, std::bind(&GameServer::OnEnterWorldNext, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::world_Request_Move, std::bind(&GameServer::OnMove, this, ph::_1, ph::_2)));
	message_handlers_.insert(std::make_pair(proto::MessageT::world_Request_Attack, std::bind(&GameServer::OnAttack, this, ph::_1, ph::_2)));
}
