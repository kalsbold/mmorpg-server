#include "stdafx.h"
#include "EventHandlers.h"

namespace mmog {

	void EventHandlers::OnSessionOpen(const Ptr<Session>& session)
	{

	}

	void EventHandlers::OnSessionClose(const Ptr<Session>& session, CloseReason reason)
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

	void EventHandlers::RegisterEventHandlers()
	{
		assert(server != nullptr);


	}

}


