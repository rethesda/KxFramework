#pragma once
#include "Common.h"
class wxMouseState;
class wxKeyboardState;

namespace kxf::wxWidgets
{
	wxMouseState GetMouseState() noexcept;
	wxKeyboardState GetKeyboardState() noexcept;
}
