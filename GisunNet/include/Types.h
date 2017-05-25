#pragma once

#include <cstdint>
#include <cassert>
#include <memory>
#include <string>
#include <functional>
#include <exception>
#include <atomic>
#include <thread>
#include <boost/log/trivial.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/asio.hpp>

namespace gisun {

	template<typename T>
	using Ptr = std::shared_ptr<T>;

	template<typename T>
	using WeakPtr = std::weak_ptr<T>;

	using boost::uuids::uuid;

	using boost::system::error_code;

	namespace asio = boost::asio;

} // namespace gisun

#define DECLARE_CLASS_PTR(CLS) \
  static const gisun::Ptr<CLS> NullPtr; \
  static const gisun::WeakPtr<CLS> NullWeakPtr;

#define DEFINE_CLASS_PTR(CLS) \
  const gisun::Ptr<CLS> CLS::NullPtr; \
  const gisun::WeakPtr<CLS> CLS::NullWeakPtr;