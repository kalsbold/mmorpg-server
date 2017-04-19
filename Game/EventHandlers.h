#pragma once
#include "GameServer.h"

namespace mmog {

	class EventHandlers
	{
	public:
		EventHandlers(GameServer* server)
			: server_(server)
		{

		}
		~EventHandlers() {}

		void RegisterEventHandlers();
	
	private:
		void OnSessionOpen(const Ptr<Session>& session);
		void OnSessionClose(const Ptr<Session>& session, CloseReason reason);
		
		GameServer* server_;
	};

}
