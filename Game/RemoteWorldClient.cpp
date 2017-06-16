#include "stdafx.h"
#include "RemoteWorldClient.h"
#include "WorldServer.h"
#include "World.h"
#include "PlayerCharacter.h"


RemoteWorldClient::RemoteWorldClient(const Ptr<net::Session>& net_session, WorldServer * owner)
	: RemoteClient(net_session)
	, owner_(owner)
	, state_(State::Connected)
	, disposed_(false)
{
	assert(owner != nullptr);
}

RemoteWorldClient::~RemoteWorldClient()
{
	Dispose();
}

inline void RemoteWorldClient::UpdateToDB()
{
	// 케릭터 상태 DB Update.
	if (character_)
		character_->UpdateToDB(GetDB());
}

// 종료 처리. 상태 DB Update 등 을 한다.

inline void RemoteWorldClient::Dispose()
{
	bool exp = false;
	if (!disposed_.compare_exchange_strong(exp, true))
		return;

	Zone* zone = character_->GetLocationZone();
	if (zone)
	{
		zone->GetWorld()->Dispatch([zone, character = character_] {
			zone->Leave(character);
		});
	}

	UpdateToDB();
}

inline const Ptr<MySQLPool>& RemoteWorldClient::GetDB()
{
	return owner_->GetDB();
}
