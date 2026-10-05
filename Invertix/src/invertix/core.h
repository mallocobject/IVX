#pragma once

#ifdef IVX_PLATFORM_WINDOWS
#ifdef IVX_BUILD_DLL
#define IVX_API __declspec(dllexport)
#else
#define IVX_API __declspec(dllimport)
#endif
#else
#error "Invertix only supports Windows!"
#endif

#ifdef IVX_ENABLE_ASSERTS
#define IVX_ASSERT(x, ...) { if(!(x)) { IVX_ERROR("Assertion Failed: {}", __VA_ARGS__); __debugbreak(); } }
#define IVX_CORE_ASSERT(x, ...) { if(!(x)) { IVX_CORE_ERROR("Assertion Failed: {}", __VA_ARGS__); __debugbreak(); } }
#else
#define IVX_ASSERT(x, ...)
#define IVX_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define IVX_BIND_EVENT_FN(Arg, Fn) [this](Arg& e){ return Fn(e); } 