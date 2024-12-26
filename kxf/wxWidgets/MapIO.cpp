#include "kxf-pch.h"
#include "MapIO.h"
#include <wx/stream.h> 
#include <wx/filefn.h> 

namespace kxf::wxWidgets
{
	std::optional<wxSeekMode> MapSeekMode(IOStreamSeek seekMode) noexcept
	{
		switch (seekMode)
		{
			case IOStreamSeek::FromStart:
			{
				return wxSeekMode::wxFromStart;
			}
			case IOStreamSeek::FromCurrent:
			{
				return wxSeekMode::wxFromCurrent;
			}
			case IOStreamSeek::FromEnd:
			{
				return wxSeekMode::wxFromEnd;
			}
		};
		return {};
	}
	std::optional<IOStreamSeek> MapSeekMode(wxSeekMode seekMode) noexcept
	{
		switch (seekMode)
		{
			case wxSeekMode::wxFromStart:
			{
				return IOStreamSeek::FromStart;
			}
			case wxSeekMode::wxFromCurrent:
			{
				return IOStreamSeek::FromCurrent;
			}
			case wxSeekMode::wxFromEnd:
			{
				return IOStreamSeek::FromEnd;
			}
		};
		return {};
	}
}
