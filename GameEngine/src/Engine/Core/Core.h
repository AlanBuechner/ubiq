#pragma once
#include <memory>
#include <filesystem>
#include "Engine/Math/Math.h"
#include "Reflection.h"
#include "Utils/Common.h"
#include "Utils/Utils.h"
#include "Utils/Types.h"
#include "Utils/Vector.h"
#include "Utils/Performance.h"
#include "Engine/Core/Logging/Log.h"

#define ENABLE_ASSERTS

#ifdef _WIN32
	#ifdef _WIN64
		#define PLATFORM_WINDOWS
		#include "Platform/Windows/Win.h"
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

#ifdef ENABLE_ASSERTS
	#define CORE_ASSERT_FUNC(x, func, msg, ...) {if(!(x)) {CORE_ERROR((std::string("Assertion Failed: ") + (msg)).c_str(), __VA_ARGS__); func;}}
	#define CORE_ASSERT(x, msg, ...) CORE_ASSERT_FUNC(x, __debugbreak(), msg, __VA_ARGS__)
	#define CORE_ASSERT_BREAK(x, msg, ...) CORE_ASSERT_FUNC(x, break, msg, __VA_ARGS__)
	#define CORE_ASSERT_CONTINUE(x, msg, ...) CORE_ASSERT_FUNC(x, continue, msg, __VA_ARGS__)
	#define CORE_ASSERT_RETURN(x, msg, ...) CORE_ASSERT_FUNC(x, return, msg, __VA_ARGS__)
#else
	#define CORE_ASSERT_FUNC(...)
	#define CORE_ASSERT(...)
	#define CORE_ASSERT_BREAK(...)
	#define CORE_ASSERT_CONTINUE(...)
	#define CORE_ASSERT_RETURN(...)
#endif // ENABLE_ASSERTS


#define BIT(x) (1 << x)

#define BIND_EVENT_FN(x) std::bind(x, this, std::placeholders::_1)
#define BIND_EVENT_FN_EXTERN(x, p) std::bind(x, p, std::placeholders::_1)

#define MEM_KiB(kib)	(kib * 1024)
#define MEM_MiB(mib)	MEM_KiB(mib * 1024)
#define MEM_GiB(gib)	MEM_MiB(gib * 1024)

#define DISABLE_COPY(type) type(const type&) = delete;

#define TOKEN_PASTE(x, y) x##y
#define CAT(x,y) TOKEN_PASTE(x,y)

namespace Engine
{
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Scope = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Weak = std::weak_ptr<T>;
}

namespace fs = std::filesystem;
