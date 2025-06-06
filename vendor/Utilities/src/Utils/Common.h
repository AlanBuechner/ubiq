#pragma once

#if defined(STRIP_CLANG)
	#define DISABLE_OPS
#else
	#define DISABLE_OPS __attribute__((optnone))
#endif

#define TRACY_ENABLE
#include "tracy/Tracy.hpp"

namespace Utils
{

}
