#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include <boost/asio.hpp>

namespace gisunnet
{	
	// A pool of io_service.
	class IoServicePool
	{
	public:
		// Construct
		IoServicePool(size_t thread_count);
		~IoServicePool();

		// Stop all io_service.
		void Stop();
		// Wait all io_service.
		void Wait();

		// Get an io_service.
		boost::asio::io_service& PickIoService()
		{
			return ChoosePowerOfTwo();
		}

	private:
		// 순차적으로 선택함
		boost::asio::io_service& ChoosePowerOfTwo()
		{
			return *io_services_[idx_++ & (io_services_.size() - 1)];
		}

		std::atomic<int> idx_;
		std::vector<std::unique_ptr<boost::asio::io_service>> io_services_;
		std::vector<std::unique_ptr<boost::asio::io_service::work>> works_;
		std::vector<std::thread> threads_;
	};
} // namespace gisunnet
