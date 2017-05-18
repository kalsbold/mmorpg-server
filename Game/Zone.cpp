#include "stdafx.h"
#include "Zone.h"

namespace mmog{

inline Zone::Zone(const uuid & uuid, asio::io_service & ios, World * world)
	: uuid_(uuid)
	, strand_(ios)
	, world_(world)
{

}

inline Zone::~Zone() {}

inline Ptr<GameObject> Zone::GetCharacter(uuid uuid)
{
	auto iter = characters_.find(uuid);
	if (iter == characters_.end())
		return nullptr;

	return iter->second;
}

inline void Zone::AddCharacter(uuid uuid, Ptr<mmog::Character> c)
{
	characters_.insert(make_pair(uuid, c));
}

inline void Zone::RemoveCharacter(uuid uuid)
{
	characters_.erase(uuid);
}

}


