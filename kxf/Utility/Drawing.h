#pragma once
#include "kxf/Common.hpp"
#include "kxf/Drawing/Geometry.h"
struct tagRECT;

namespace kxf::Utility
{
	void FromWindowsRect(const tagRECT& winRect, Rect& rect) noexcept;
	Rect FromWindowsRect(const tagRECT& winRect) noexcept;

	void ToWindowsRect(const Rect& rect, tagRECT& winRect) noexcept;
	tagRECT ToWindowsRect(const Rect& rect) noexcept;
}
