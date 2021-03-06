#pragma once

#include <thread>
#include <atomic>
#include <vector>
#include "Types.h"

namespace gisun {
namespace net {

	// Run the io_service object's event processing loop.
	class IoServiceLoop
	{
	public:
		IoServiceLoop(const IoServiceLoop&) = delete;
		IoServiceLoop& operator=(const IoServiceLoop&) = delete;

		explicit IoServiceLoop(size_t count = 1, bool ios_pool = false);
		~IoServiceLoop();

		// Stop io_service.
		void Stop();
		// Wait io_service.
		void Wait();

		// Get an io_service.
		asio::io_service& GetIoService()
		{
			return ChoosePowerOfTwo();
		}

	private:
		// 순차적으로 선택함
		asio::io_service& ChoosePowerOfTwo()
		{
			return *io_services_[idx_++ & (io_services_.size() - 1)];
		}

		std::atomic<size_t> idx_;
		std::vector<std::unique_ptr<asio::io_service>> io_services_;
		std::vector<std::unique_ptr<asio::io_service::work>> works_;
		std::vector<std::thread> threads_;
	};

} // namespace net
} // namespace gisun
