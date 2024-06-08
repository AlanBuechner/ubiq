#pragma once

#if defined(STRIP_CLANG)
	#define DISABLE_OPS
#else
	#define DISABLE_OPS __attribute__((optnone))
#endif

namespace Utils
{

}
