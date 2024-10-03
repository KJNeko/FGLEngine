//
// Created by kj16609 on 5/22/24.
//

#pragma once

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
#include <spdlog/spdlog.h>
#pragma GCC diagnostic pop

#include "formatters/filesystem.hpp"
#include "formatters/matrix.hpp"
#include "formatters/glm.hpp"

namespace fgl::engine::log
{
	using namespace ::spdlog;
}