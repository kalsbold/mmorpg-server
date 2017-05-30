#pragma once
#include <iostream>

class ServerBase
{
public:
	ServerBase() {}
	virtual ~ServerBase() {}

	std::string GetName() { return name_; }
	void SetName(const std::string& name) { name_ = name; }
		
	virtual void Run() {}
	virtual void Stop() {};

private:
	std::string name_;
};
