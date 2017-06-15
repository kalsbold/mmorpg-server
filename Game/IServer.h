#pragma once
#include <iostream>

class IServer
{
public:
	virtual ~IServer() {};
	virtual std::string GetName() = 0;
	virtual void SetName(const std::string& name) = 0;
	virtual void Run() = 0;
	virtual void Stop() = 0;
};
