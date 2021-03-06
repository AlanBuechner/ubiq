#pragma once
#include <memory>

#define ENABLE_ASSERTS

#ifdef _WIN32
	#ifdef _WIN64
		#define PLATFORM_WINDOWS
	#else
		#error "x86 Buillds are not sapported"
	#endif // _WIN64
#elif defined(__APPLE__) || defined(__MACH__)
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR == 1
		#error "IOS simulator is not sapported"
	#elif TARGET_OS_IPHONE == 1
		#define PLATFORM_IOS
		#error "IOS is not sapported"
	#elif TARGET_OS_MAC == 1
		#define PLATFORM_MACOS
		#error "MacOS is not sapported"
	#endif
#elif defined(__ANDROID__)
	#define PLATFORM_ANDROID
	#error "Android is not sapported"
#elif defined(__linux__)
	#define PLATFORM_LINUX
	#error "Linux is not sapported"
#else
	#error "Unknown Platform!"
#endif

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

namespace Engine
{
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T>
	using Scope = std::unique_ptr<T>;

	using byte = unsigned char;
}