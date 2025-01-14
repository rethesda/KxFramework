#pragma once
#include "kxf/Common.hpp"

#ifdef KXF_DYNAMIC_LIBRARY
	#ifdef KXF_LIBRARY_COMPRESSION
		#define KXF_API_COMPRESSION __declspec(dllexport)
	#else
		#define KXF_API_COMPRESSION __declspec(dllimport)
	#endif
#elif defined KXF_STATIC_LIBRARY
	#define KXF_API_COMPRESSION
#endif
