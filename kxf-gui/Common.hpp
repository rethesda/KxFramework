#pragma once
#ifdef KXF_DYNAMIC_LIBRARY
	#ifdef KXF_LIBRARY_GUI
		#define KXF_API_GUI __declspec(dllexport)
	#else
		#define KXF_API_GUI __declspec(dllimport)
	#endif
#elif defined KXF_STATIC_LIBRARY
	#define KXF_API_GUI
#endif

#include "kxf/Common.hpp"
#include "kxf/wxWidgets/Setup.h"
#include "kxf/wxWidgets/Setup-IncludeBasic.h"
