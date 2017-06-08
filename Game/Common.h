#pragma once
#include <iostream>
#include <chrono>
#include <type_traits>
#include <GisunNet.h>
#include "TypeDef.h"

using namespace gisun;

template<typename T>
using UPtr = std::unique_ptr<T>;