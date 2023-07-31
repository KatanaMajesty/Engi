#pragma once

#include <cstdlib>

// TODO: Replace _WIN32 and WIN32
#if defined(_WIN32) || defined(WIN32)
	#define ENGI_API _declspec(dllexport)
#endif

#define ENGI_MULTITHREADING

#define BREAK __debugbreak();

#define ENGI_ALWAYS_ASSERT(expression, ...) \
	if (!(expression)) \
	{ \
		BREAK; \
		std::abort(); \
	}

#ifdef NDEBUG
	#define ENGI_ASSERT(...)
#else
	#define ENGI_ASSERT(expression, ...) ENGI_ALWAYS_ASSERT(expression, __VA_ARGS__);
#endif
