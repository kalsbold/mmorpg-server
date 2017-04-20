#include "stdafx.h"
#include "EventHandlers.h"

namespace mmog {

	using namespace std;
	using namespace protocol;

	void EventHandlers::OnSessionOpen(const Ptr<Session>& session)
	{

	}

	void EventHandlers::OnSessionClose(const Ptr<Session>& session, CloseReason reason)
	{
		
	}

	void EventHandlers::OnLogin(const Ptr<Session>& session, const protocol::NetMessage * net_message)
	{
	}

	void EventHandlers::OnJoin(const Ptr<Session>& session, const protocol::NetMessage * net_message)
	{
	}

	void EventHandlers::OnCreateCharacter(const Ptr<Session>& session, const protocol::NetMessage * net_message)
	{
	}

	void EventHandlers::OnCharacterList(const Ptr<Session>& session, const protocol::NetMessage * net_message)
	{
	}

	void EventHandlers::OnDeleteCharacter(const Ptr<Session>& session, const protocol::NetMessage * net_message)
	{
	}

	void EventHandlers::OnEnterGame(const Ptr<Session>& session, const protocol::NetMessage * net_message)
	{
	}

	void EventHandlers::RegisterEventHandlers()
	{
		
		BOOST_ASSERT(server_ != nullptr);

		server_->RegisterSessionOpenHandler(bind(&EventHandlers::OnSessionOpen, this, placeholders::_1));
		server_->RegisterSessionCloseHandler(bind(&EventHandlers::OnSessionClose, this, placeholders::_1, placeholders::_2));
		
		server_->RegisterMessageHandler(MessageT_RequestLogin, std::bind(&EventHandlers::OnLogin, this, std::placeholders::_1, std::placeholders::_2));
		server_->RegisterMessageHandler(MessageT_RequestJoin, std::bind(&EventHandlers::OnJoin, this, std::placeholders::_1, std::placeholders::_2));
		server_->RegisterMessageHandler(MessageT_RequestCreateCharacter, std::bind(&EventHandlers::OnCreateCharacter, this, std::placeholders::_1, std::placeholders::_2));
		server_->RegisterMessageHandler(MessageT_RequestCharacterList, std::bind(&EventHandlers::OnCharacterList, this, std::placeholders::_1, std::placeholders::_2));
		server_->RegisterMessageHandler(MessageT_RequestDeleteCharacter, std::bind(&EventHandlers::OnDeleteCharacter, this, std::placeholders::_1, std::placeholders::_2));
		server_->RegisterMessageHandler(MessageT_RequestEnterGame, std::bind(&EventHandlers::OnEnterGame, this, std::placeholders::_1, std::placeholders::_2));
	}

}


