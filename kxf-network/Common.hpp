#pragma once
#include "kxf/Common.hpp"

#ifdef KXF_DYNAMIC_LIBRARY
	#ifdef KXF_LIBRARY_NETWORK
		#define KXF_API_NETWORK __declspec(dllexport)
	#else
		#define KXF_API_NETWORK __declspec(dllimport)
	#endif
#elif defined KXF_STATIC_LIBRARY
	#define KXF_API_NETWORK
#endif
