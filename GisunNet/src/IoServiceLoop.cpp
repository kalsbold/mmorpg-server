#include "network/IoServiceLoop.h"

namespace gisun {
namespace net {

IoServiceLoop::IoServiceLoop(size_t count, bool ios_pool)
	: idx_(0)
{
	if (count == 0)
		throw std::invalid_argument("count 0");

	if (ios_pool)
	{
		for (int i = 0; i < count; i++)
		{
            auto ios = std::make_unique<boost::asio::io_service>();
			works_.emplace_back(std::make_unique<boost::asio::io_service::work>(*ios));
			threads_.emplace_back(std::thread([this, ios = ios.get()]()
			{
				try
				{
                    ios->run();
				}
				catch (const std::exception& e)
				{

					BOOST_LOG_TRIVIAL(error) << "io_service run exception! : " << e.what();
				}
			}));
            io_services_.emplace_back(std::move(ios));
		}
	}
	else
	{
        auto ios = std::make_unique<boost::asio::io_service>();
		works_.emplace_back(std::make_unique<boost::asio::io_service::work>(*ios));

		for (int i = 0; i < count; i++)
		{
			threads_.emplace_back(std::thread([this, ios = ios.get()]()
			{
				try
				{
					ios->run();
				}
				catch (const std::exception& e)
				{

					BOOST_LOG_TRIVIAL(error) << "io_service run exception! : " << e.what();
				}
			}));
		}
        io_services_.emplace_back(std::move(ios));
	}

	BOOST_LOG_TRIVIAL(info) << "Run IoServiceLoop. count:" << count;
}

IoServiceLoop::~IoServiceLoop()
{
	Stop();
	Wait();
}

void IoServiceLoop::Stop()
{
    works_.clear();

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
		if (thread.joinable())
			thread.join();
	}
}

} // namespace net
} // namespace gisun