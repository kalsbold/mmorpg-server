#include "gisunnet/network/IoServicePool.h"

namespace gisunnet {

IoServicePool::IoServicePool(size_t thread_count)
	: idx_(0)
{
	if (thread_count == 0)
		throw std::runtime_error("thread_count is 0");

	for (int i = 0; i < thread_count; i++)
	{
		io_services_.emplace_back(std::make_unique<boost::asio::io_service>());
		works_.emplace_back(std::make_unique<boost::asio::io_service::work>(*io_services_[i]));
		threads_.emplace_back(std::thread([this, i]() { (*io_services_[i]).run(); }));
	}
}

IoServicePool::~IoServicePool()
{
	Stop();
	for (auto& thread : threads_)
	{
		thread.detach();
	}
}

void IoServicePool::Stop()
{
	for (auto& ios : io_services_)
	{
		ios->stop();
	}
}

void IoServicePool::Wait()
{
	for (auto& thread : threads_)
	{
		if(thread.joinable())
			thread.join();
	}
}

} // namespace gisunnet