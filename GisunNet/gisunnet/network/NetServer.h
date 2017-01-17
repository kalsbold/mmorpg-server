#pragma once
#include <memory>
#include <thread>
#include <mutex>
#include <boost/serialization/singleton.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include "TcpServer.h"
#include "NetMessage.h"
#include "IoServicePool.h"
#include "NetSession.h"


namespace gisunnet
{

	class NetIoServicePool : public boost::serialization::singleton<NetIoServicePool>
	{
		friend class boost::serialization::singleton<NetIoServicePool>;
	public:
		using IoServicePoolPtr = std::shared_ptr<IoServicePool>;

		NetIoServicePool()
		{
			auto num_cpus = std::thread::hardware_concurrency();
			ios_pool_ = std::make_shared<IoServicePool>(num_cpus);
		}
		~NetIoServicePool()
		{
			Stop();
		}

		static NetIoServicePool& Instance()
		{
			return boost::serialization::singleton<NetIoServicePool>::get_mutable_instance();
		}

		IoServicePoolPtr GetIoServicePool()
		{
			return ios_pool_;
		}

		void Stop()
		{
			ios_pool_->Stop();
		}

	private:
		std::shared_ptr<IoServicePool> ios_pool_;
	};
	//===============================================================================


	class NetServer : std::enable_shared_from_this<NetServer>
	{
	public:
		using UID = boost::uuids::uuid;
		using MessageID = int;

		enum class State
		{
			Ready = 0,
			Start,
			Stop,
		};

		~NetServer()
		{
			Stop();
		}

		static std::shared_ptr<NetServer> Create()
		{
			std::shared_ptr<NetServer> s = std::shared_ptr<NetServer>(new NetServer());
			return s;
		}

		void Start(int port)
		{
			std::lock_guard<std::mutex> lock(mutex_);

			if (!(state_ == State::Ready))
			{
				return;
			}

			server_->ConnectHandler = [this](TcpServer::TransportPtr& transport) { this->HandleConnect(transport); };
			server_->Listen(port, boost::asio::ip::tcp::v4());
			state_ = State::Start;
		}

		void Stop()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			server_->Close();
			state_ = State::Stop;
			// 技记 甘阑 厚款促.
			sessions_.clear();
		}

		State GetState()
		{
			std::lock_guard<std::mutex> lock(mutex_);
			return state_;
		}
		
		std::map<MessageID, std::function<void(NetMessage&)>> MessageHandler;

	private:
		NetServer()
		{
			server_ = std::make_unique<TcpServer>(NetIoServicePool::Instance().GetIoServicePool());
		}

		void HandleConnect(TcpServer::TransportPtr& transport)
		{
			// Unique ID 且寸.
			UID uid = boost::uuids::random_generator()();
			// 技记 按眉 积己.
			auto session = std::make_unique<NetSession>(uid, transport);
			session->StopHandler = [this](const UID& uid) { HandleStop(uid); };

			std::lock_guard<std::mutex> lock(mutex_);
			// 技记 甘俊 眠啊.
			sessions_.emplace(uid, std::move(session));
		}

		void HandleStop(const UID& uid)
		{
			std::lock_guard<std::mutex> lock(mutex_);
			sessions_.erase(uid);
		}

		std::mutex mutex_;
		State state_ = State::Ready;;
		std::unique_ptr<TcpServer> server_;
		std::map<UID, std::unique_ptr<NetSession>> sessions_;
	};
}