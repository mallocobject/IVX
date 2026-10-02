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