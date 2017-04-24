#pragma once
#include "TypeDef.h"
#include "Singleton.h"
#include "DatabaseEntity.h"
#include "MySQL.h"

namespace mmog {

	class MapData : public Singleton<MapData>
	{
	public:

		static void Load()
		{
			MapData& instance = GetInstance();

		}

	private:
		std::vector<std::pair<int, Map>> map_data_;
	};
}

