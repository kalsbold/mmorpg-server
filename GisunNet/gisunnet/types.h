#pragma once

#include <cstdint>
#include <memory>
#include <functional>
#include <string>
#include <vector>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio.hpp>

#include <gisunnet/network/ByteBuffer.h>

namespace gisunnet {

	template<typename T>
	using Ptr = std::shared_ptr<T>;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	using std::function;

	using uuid = boost::uuids::uuid;

	using string = std::string;

	using error_code = boost::system::error_code;

	// 기본 버퍼 타입.
	using Buffer = ByteBuffer<>;
}

