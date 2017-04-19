#pragma once

#include <map>
#include <mutex>
#include "TypeDef.h"

namespace mmog {

	class ServerBase
	{
	public:
		string GetName() { return name_; }
		void SetName(const string& name) { name_ = name; }
		virtual void Run() = 0;
		virtual void Stop() = 0;

	private:
		string name_;
	};
}
