#pragma once
#include "kxf/IO/Common.h"
enum wxSeekMode: int;
enum wxStreamError: int;

namespace kxf::wxWidgets
{
	std::optional<wxSeekMode> MapSeekMode(IOStreamSeek seekMode) noexcept;
	std::optional<IOStreamSeek> MapSeekMode(wxSeekMode seekMode) noexcept;
}
