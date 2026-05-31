#include "kxf-pch.h"
#include "LegacyVolume.h"
#include "FSPath.h"
#include "kxf/Utility/Common.h"

#include <Windows.h>
#include "kxf/Win32/UndefMacros.h"

namespace
{
	constexpr char g_FirstLegacyVolume = 'A';
	constexpr char g_LastLegacyVolume = 'Z';
	constexpr size_t g_LegacyVolumesCount = g_LastLegacyVolume - g_FirstLegacyVolume + 1;

	constexpr bool IsDriveLetterValid(char c)
	{
		return c >= g_FirstLegacyVolume && c <= g_LastLegacyVolume;
	}
}

namespace kxf
{
	CallbackResult<void> LegacyVolume::EnumVolumes(CallbackFunction<LegacyVolume> func)
	{
		auto driveMask = ::GetLogicalDrives();
		for (size_t i = 0; i <= g_LegacyVolumesCount; i++)
		{
			if (driveMask & (1 << i))
			{
				if (func.Invoke(FromIndex(static_cast<int>(i))).ShouldTerminate())
				{
					break;
				}
			}
		}
		return func.Finalize();
	}

	void LegacyVolume::AssignFromChar(const UniChar& value)
	{
		m_Drive = 0;
		if (auto c = value.ToASCII())
		{
			c = std::toupper(*c);
			if (IsDriveLetterValid(*c))
			{
				m_Drive = *c;
			}
		}
	}
	void LegacyVolume::AssignFromIndex(int index)
	{
		m_Drive = 0;
		if (index >= 0 && index <= g_LegacyVolumesCount)
		{
			AssignFromChar(index + g_FirstLegacyVolume);
		}
	}
	String LegacyVolume::DoGetPath() const
	{
		if (IsValid())
		{
			XChar disk[] = kxfS("\0:\\");
			disk[0] = m_Drive;

			return disk;
		}
		return {};
	}

	bool LegacyVolume::IsValid() const
	{
		return IsDriveLetterValid(m_Drive);
	}
	bool LegacyVolume::DoesExist() const
	{
		if (IsValid())
		{
			String path = DoGetPath();
			return ::GetDriveTypeW(path.wc_str()) != DRIVE_NO_ROOT_DIR;
		}
		return false;
	}

	FSPath LegacyVolume::GetPath() const
	{
		return DoGetPath();
	}
	int LegacyVolume::GetIndex() const
	{
		if (IsValid())
		{
			return m_Drive - g_FirstLegacyVolume;
		}
		return -1;
	}
	char LegacyVolume::GetChar() const
	{
		if (IsValid())
		{
			return m_Drive;
		}
		return 0;
	}
}
