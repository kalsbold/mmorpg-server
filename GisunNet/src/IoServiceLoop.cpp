#include "gisunnet/network/IoServiceLoop.h"

namespace gisunnet {

IoServiceLoop::IoServiceLoop(size_t thread_count, bool ios_pool)
	: idx_(0)
{
	if (thread_count == 0)
		throw std::invalid_argument("thread_count 0");

	if (ios_pool)
	{
		for (int i = 0; i < thread_count; i++)
		{
			io_services_.emplace_back(std::make_unique<boost::asio::io_service>());
			works_.emplace_back(std::make_unique<boost::asio::io_service::work>(*io_services_[i]));
			threads_.emplace_back(std::thread([this, i]()
			{
				try
				{
					io_services_[i]->run();
				}
				catch (const std::exception& e)
				{

					BOOST_LOG_TRIVIAL(error) << "io_service run exception! : " << e.what();
				}
			}));
		}
	}
	else
	{
		io_services_.emplace_back(std::make_unique<boost::asio::io_service>());
		works_.emplace_back(std::make_unique<boost::asio::io_service::work>(*io_services_[0]));

		for (int i = 0; i < thread_count; i++)
		{
			threads_.emplace_back(std::thread([this]()
			{
				try
				{
					io_services_[0]->run();
				}
				catch (const std::exception& e)
				{

					BOOST_LOG_TRIVIAL(error) << "io_service run exception! : " << e.what();
				}
			}));
		}
	}

	BOOST_LOG_TRIVIAL(info) << "Run IoServiceLoop. thread_count:" << thread_count;
}

IoServiceLoop::~IoServiceLoop()
{
	Stop();
	Wait();
}

void IoServiceLoop::Stop()
{
	for (auto& ios : io_services_)
	{
		ios->stop();
	}
	BOOST_LOG_TRIVIAL(info) << "Stop IoServiceLoop";
}

void IoServiceLoop::Wait()
{
	for (auto& thread : threads_)
	{
		if(thread.joinable())
			thread.join();
	}
}

} // namespace gisunnet