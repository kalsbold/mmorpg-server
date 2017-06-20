#include "stdafx.h"
#include "Hero.h"
#include "Zone.h"

Hero::Hero(const uuid & entity_id, RemoteClient * rc, const db::Hero & db_data)
	: Actor(entity_id)
	, rc_(rc)
{
	InitAttribute(db_data);
}

Hero::~Hero() {}

void Hero::SetLocationZone(Zone * zone)
{
	Actor::SetLocationZone(zone);
	if (zone == nullptr) return;

	map_id_ = zone->MapID();
}
