#pragma once
#include "kxf/Common.hpp"

#ifdef KXF_DYNAMIC_LIBRARY
	#ifdef KXF_LIBRARY_CRYPTO
		#define KXF_API_CRYPTO __declspec(dllexport)
	#else
		#define KXF_API_CRYPTO __declspec(dllimport)
	#endif
#elif defined KXF_STATIC_LIBRARY
	#define KXF_API_CRYPTO
#endif
