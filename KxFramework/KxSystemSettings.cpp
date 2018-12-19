/*
Copyright � 2018 Kerber. All rights reserved.

You should have received a copy of the GNU LGPL v3
along with KxFramework. If not, see https://www.gnu.org/licenses/lgpl-3.0.html.
*/
#include "KxStdAfx.h"
#include "KxFramework/KxSystemSettings.h"
#include "KxFramework/KxSystem.h"
#include "KxFramework/KxRegistry.h"
#include "KxFramework/KxColor.h"

KxColor KxSystemSettings::GetColor(wxSystemColour index)
{
	return wxSystemSettings::GetColour(index);
}
int KxSystemSettings::GetMetric(wxSystemMetric index, const wxWindow* window)
{
	return wxSystemSettings::GetMetric(index, const_cast<wxWindow*>(window));
}
KxStringVector KxSystemSettings::GetSoundsList()
{
	return KxRegistry::GetKeyNames(KxREG_HKEY_CURRENT_USER, "AppEvents\\EventLabels");
}
KxSystemSettings::DisplayInfo KxSystemSettings::GetDisplayInfo()
{
	HDC desktopDC = ::GetDC(nullptr);
	DisplayInfo info;

	info.Width = ::GetDeviceCaps(desktopDC, DESKTOPHORZRES);
	info.Height = ::GetDeviceCaps(desktopDC, DESKTOPVERTRES);
	info.Depth = ::GetDeviceCaps(desktopDC, BITSPIXEL);
	info.Frequency = ::GetDeviceCaps(desktopDC, VREFRESH);

	::ReleaseDC(nullptr, desktopDC);
	return info;
}
KxSystemSettings::VideoAdapterList KxSystemSettings::EnumVideoAdapters()
{
	VideoAdapterList list;
	bool isSuccess = false;
	DWORD index = 0;
	DISPLAY_DEVICE info = {0};
	info.cb = sizeof(info);

	std::unordered_set<size_t> hash;
	std::hash<std::wstring_view> Hasher;
	do
	{
		isSuccess = ::EnumDisplayDevicesW(nullptr, index, &info, 0);
		if (info.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP || info.StateFlags == 0 && wcscmp(info.DeviceString, L"") != 0)
		{
			size_t hashValue = Hasher(info.DeviceString);
			if (hash.count(hashValue) == 0)
			{
				hash.insert(hashValue);
				list.push_back(info);
			}
		}
		index++;
	}
	while (isSuccess);
	return list;
}
KxSystemSettings::VideoModeList KxSystemSettings::EnumVideoModes(const wxString& deviceName)
{
	VideoModeList list;

	DWORD index = 0;
	DEVMODEW info = {0};
	info.dmSize = sizeof(info);

	while (::EnumDisplaySettingsW(deviceName.IsEmpty() ? nullptr : deviceName.wc_str(), index, &info))
	{
		list.push_back(VideoMode{info.dmPelsWidth, info.dmPelsHeight, info.dmBitsPerPel, info.dmDisplayFrequency});
		index++;
	}
	return list;
}
