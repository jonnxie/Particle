#pragma once

#ifdef SHATTER_PLATPORM_WINDOWS
	#ifdef SHATTER_BUILD_DLL
		#define SHATTER_API __declspec(dllexport)
	#else
		#define SHATTER_API __declspec(dllimport)
	#endif // SHATTER_BUILD_DLL
#else
	#error  Shatter only support Windows!
#endif
