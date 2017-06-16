#include "stdafx.h"
#include "PlayerCharacter.h"
#include "Zone.h"

PlayerCharacter::PlayerCharacter(const uuid & entity_id, RemoteClient * rc, const db::Character & db_data)
	: Actor(entity_id)
	, rc_(rc)
{
	InitAttribute(db_data);
}

PlayerCharacter::~PlayerCharacter() {}

void PlayerCharacter::SetLocationZone(Zone * zone)
{
	Actor::SetLocationZone(zone);
	if (zone == nullptr) return;

	map_id_ = zone->MapID();
}
