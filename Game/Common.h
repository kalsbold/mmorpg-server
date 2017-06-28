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
using timer = boost::asio::high_resolution_timer;

using boost::asio::strand;
using boost::uuids::random_generator;

using namespace AO::Vector3;