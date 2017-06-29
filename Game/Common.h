#pragma once
#include <iostream>
#include <chrono>
#include <type_traits>
#include <algorithm>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <tuple>
#include <boost/signals2.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/string_generator.hpp>
#include <boost/functional/hash.hpp>
#include <GisunNet.h>
#include <flatbuffers\flatbuffers.h>
#include "Vector3.h"

using namespace gisun;

template<typename T>
using UPtr = std::unique_ptr<T>;

using namespace std::chrono_literals;

// Update frame time step.(20fps)
constexpr std::chrono::nanoseconds TIME_STEP(50ms);

using double_seconds = std::chrono::duration<double>;

// The clock type.
using clock_type = std::chrono::high_resolution_clock;
// The duration type of the clock.
using duration = clock_type::duration;
// The time point type of the clock.
using time_point = clock_type::time_point;
// Timer type
using timer_type = boost::asio::high_resolution_timer;

using boost::asio::strand;
using boost::uuids::random_generator;

using boost::uuids::uuid;

namespace std
{
    template<>
    struct hash<boost::uuids::uuid>
    {
        size_t operator () (const boost::uuids::uuid& uid) const
        {
            return boost::hash<boost::uuids::uuid>()(uid);
        }
    };
}
using uuid_hasher = std::hash<boost::uuids::uuid>;

using namespace AO::Vector3;