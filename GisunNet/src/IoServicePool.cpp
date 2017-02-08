#include "gisunnet/network/IoServicePool.h"

namespace gisunnet {

IoServicePool::IoServicePool(size_t thread_count)
	: idx_(0)
{
	if (thread_count == 0)
		throw std::invalid_argument("thread_count : " + thread_count);

	for (int i = 0; i < thread_count; i++)
	{
		io_services_.emplace_back(std::make_unique<boost::asio::io_service>());
		works_.emplace_back(std::make_unique<boost::asio::io_service::work>(*io_services_[i]));
		threads_.emplace_back(std::thread([this, i]() { (*io_services_[i]).run(); }));
	}

	BOOST_LOG_TRIVIAL(info) << "Run IoServicePool. thread_count:" << thread_count;
}

IoServicePool::~IoServicePool()
{
	Stop();
	Wait();
}

void IoServicePool::Stop()
{
	for (auto& ios : io_services_)
	{
		ios->stop();
	}
	BOOST_LOG_TRIVIAL(info) << "Stop IoServicePool";
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