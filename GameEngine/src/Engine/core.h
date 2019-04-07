#pragma once

#define ENABLE_ASSERTS

#ifdef PLATFORM_WINDOWS
	#ifdef DYNAMIC_LINK
		#ifdef BUILD_DLL
			#define ENGINE_API __declspec(dllexport)
		#else
			#define ENGINE_API __declspec(dllimport)
		#endif // BUILD_DLL
	#else
		#define ENGINE_API
	#endif
#else
	#error Platform not saported
#endif

#ifdef ENABLE_ASSERTS
	#define ASSERT(x, ...) {if(!(x)) {DEBUG_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
	#define CORE_ASSERT(x, ...) {if(!(x)) {CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak();}}
#else
	#define ASSERT(x, ...)
	#define CORE_ASSERT(x, ...)
#endif // ENABLE_ASSERTS


#define BIT(x) (1 << x)

#define BIND_EVENT_FN(x) std::bind(x, this, std::placeholders::_1)