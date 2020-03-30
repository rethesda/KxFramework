/*
Copyright � 2018 Kerber. All rights reserved.

You should have received a copy of the GNU LGPL v3
along with KxFramework. If not, see https://www.gnu.org/licenses/lgpl-3.0.html.
*/
#include "KxStdAfx.h"
#include "KxFramework/KxShell.h"
#include "KxFramework/KxSystem.h"
#include "KxFramework/KxFile.h"
#include "KxFramework/KxFileFinder.h"
#include "KxFramework/KxUtility.h"
#include <Kx/FileSystem/LegacyVolume.h>
#include <KnownFolders.h>
#include <winnls.h>
#include <shobjidl.h>
#include <objbase.h>
#include <objidl.h>
#include <shlguid.h>
#include <Shldisp.h>

HWND KxShell::GetOwnerHWND(wxWindow* window)
{
	window = wxGetTopLevelParent(window);

	HWND hWnd = nullptr;
	if (window)
	{
		hWnd = window->GetHandle();
	}
	else
	{
		window = wxTheApp->GetTopWindow();
		if (window)
		{
			hWnd = window->GetHandle();
		}
		else
		{
			window = wxGetTopLevelParent(wxGetActiveWindow());
			if (window)
			{
				return window->GetHandle();
			}
		}
	}

	return hWnd ? hWnd : ::GetShellWindow();
}

bool KxShell::FileOperation(const wxString& fullPath, KxFileSearchType elementType, KxShellOperationFunc func, bool useRecycleBin, bool recurse, wxWindow* window)
{
	return FileOperationEx(func, fullPath, wxEmptyString, window, recurse, elementType == KxFS_FILE, useRecycleBin);
}
bool KxShell::FileOperationEx(KxShellOperationFunc func, const wxString& from, const wxString& to, wxWindow* window, bool recurse, bool filesOnly, bool allowUndo, bool yesToAll)
{
	auto CreateZZString = [](const wxString& s)
	{
		if (!s.IsEmpty())
		{
			wxMemoryBuffer stringZZ(s.Length() * sizeof(wxChar) + 2 * sizeof(wxChar));
			stringZZ.AppendData(s.wx_str(), s.Length() * sizeof(wxChar));
			stringZZ.AppendData(L"\000", sizeof(wxChar));
			stringZZ.AppendData(L"\000", sizeof(wxChar));

			return stringZZ;
		}
		return wxMemoryBuffer(0);
	};
	
	wxMemoryBuffer fromZZ = CreateZZString(from);
	wxMemoryBuffer toZZ = CreateZZString(to);

	// Parent window
	window = wxGetTopLevelParent(window);

	// Fill the struct
	SHFILEOPSTRUCTW operationInfo = {0};
	operationInfo.hwnd = window ? window->GetHandle() : nullptr;
	operationInfo.wFunc = func;
	operationInfo.pFrom = (PCZZWSTR)fromZZ.GetData();
	operationInfo.pTo = (PCZZWSTR)toZZ.GetData();
	operationInfo.fFlags = FOF_NOCONFIRMMKDIR;
	if (!window)
	{
		operationInfo.fFlags |= FOF_NO_UI;
	}
	if (filesOnly)
	{
		operationInfo.fFlags |= FOF_FILESONLY|FOF_NORECURSION;
	}
	if (allowUndo)
	{
		operationInfo.fFlags |= FOF_ALLOWUNDO;
	}
	if (yesToAll)
	{
		operationInfo.fFlags |= FOF_NOCONFIRMATION;
	}
	if (!recurse)
	{
		operationInfo.fFlags |= FOF_NORECURSION;
	}

	// Disable parent window if UI actions is allowed
	bool disabled = false;
	if (window && !window->IsEnabled())
	{
		window->Disable();
		disabled = true;
	}

	int returnCode = ::SHFileOperationW(&operationInfo);

	// Re-enable parent window
	if (disabled)
	{
		window->Enable();
	}

	// Zero means function succeed.
	return returnCode == 0;
}

DWORD KxShell::FormatDrive(wxWindow* window, const KxFramework::LegacyVolume& drive, DWORD options, DWORD formatID)
{
	if (drive)
	{
		return ::SHFormatDrive(GetOwnerHWND(window), (UINT)drive.GetIndex(), formatID, options);
	}
	return SHFMT_ERROR;
}
bool KxShell::PinShortcut(const wxString& shortcutPath, KxShellShortcutPinMode mode)
{
	int ret = 0;
	switch (mode)
	{
		case KxSH_PIN_STARTMENU:
		{
			#pragma warning(suppress: 4311)
			#pragma warning(suppress: 4302)
			ret = (int)::ShellExecuteW(nullptr, L"startpin", shortcutPath.wc_str(), nullptr, nullptr, SW_SHOWNORMAL);
			break;
		}
		case KxSH_UNPIN_STARTMENU:
		{
			#pragma warning(suppress: 4311)
			#pragma warning(suppress: 4302)
			ret = (int)::ShellExecuteW(nullptr, L"startunpin", shortcutPath.wc_str(), nullptr, nullptr, SW_SHOWNORMAL);
			break;
		}
		case KxSH_PIN_TASKBAR:
		{
			#pragma warning(suppress: 4311)
			#pragma warning(suppress: 4302)
			ret = (int)::ShellExecuteW(nullptr, L"taskbarpin", shortcutPath.wc_str(), nullptr, nullptr, SW_SHOWNORMAL);
			break;
		}
		case KxSH_UNPIN_TASKBAR:
		{
			#pragma warning(suppress: 4311)
			#pragma warning(suppress: 4302)
			ret = (int)::ShellExecuteW(nullptr, L"taskbarunpin", shortcutPath.wc_str(), nullptr, nullptr, SW_SHOWNORMAL);
			break;
		}
	}
	return ret > 32;
}

wxString KxShell::QueryAssocString(const wxString& string, KxShellAssocQuery infoType)
{
	wxString extWithDot = string.AfterLast('.');
	if (!extWithDot.IsEmpty() && extWithDot[0] != '.')
	{
		extWithDot.Prepend(".");
	}

	DWORD length = 0;
	ASSOCF flags = ASSOCF_INIT_DEFAULTTOSTAR;
	::AssocQueryStringW(flags, (ASSOCSTR)infoType, extWithDot.wc_str(), nullptr, nullptr, &length);

	wxString out;
	if (length != 0)
	{
		::AssocQueryStringW(flags, (ASSOCSTR)infoType, extWithDot.wc_str(), nullptr, wxStringBuffer(out, length), &length);
	}
	return out;
}
wxString KxShell::GetTypeIcon(const wxString& ext, int* indexOut)
{
	wxString icon = QueryAssocString(ext, KxSH_AQ_DEFAULT_ICON);
	if (icon == "%1")
	{
		icon = wxEmptyString;
	}

	if (indexOut)
	{
		wxString index = icon.AfterLast(',');

		long value = 0;
		index.ToCLong(&value);
		*indexOut = std::abs(value);
	}

	wxString path = icon.BeforeLast(',');
	return !path.IsEmpty() ? path : icon;
}
wxString KxShell::GetLocalizedName(const wxString& objectPath, int* resourceIDOut)
{
	wxString out;
	int resourceID = 0;
	if (::SHGetLocalizedName(objectPath.wc_str(), wxStringBuffer(out, INT16_MAX), INT16_MAX, &resourceID) == S_OK)
	{
		KxUtility::SetIfNotNull(resourceIDOut, resourceID);
		return out;
	}
	return wxEmptyString;
}

wxIcon KxShell::GetFileIcon(const wxString& path, bool smallIcon)
{
	SHFILEINFOW shellInfo = {0};
	::SHGetFileInfoW(path.wc_str(), 0, &shellInfo, sizeof(shellInfo), SHGFI_ICON|(smallIcon ? SHGFI_SMALLICON : 0));
	if (shellInfo.hIcon != nullptr)
	{
		wxIcon icon;
		if (icon.CreateFromHICON(shellInfo.hIcon))
		{
			return icon;
		}
	}
	return wxNullIcon;
}
wxIcon KxShell::GetFileIcon(const KxFileItem& item, bool smallIcon)
{
	SHFILEINFOW shellInfo = {0};
	::SHGetFileInfoW(item.GetName().wc_str(), item.GetAttributes(), &shellInfo, sizeof(shellInfo), SHGFI_USEFILEATTRIBUTES|SHGFI_ICON|(smallIcon ? SHGFI_SMALLICON : 0));
	if (shellInfo.hIcon != nullptr)
	{
		wxIcon icon;
		if (icon.CreateFromHICON(shellInfo.hIcon))
		{
			return icon;
		}
	}
	return wxNullIcon;
}

bool KxShell::Execute(wxWindow* window,
					  const wxString& filePath,
					  const wxString& operation,
					  const wxString& arguments,
					  const wxString& workingFolder,
					  int windowMode,
					  bool hideUI
)
{
	SHELLEXECUTEINFOW info = {0};
	info.cbSize = sizeof(info);
	info.fMask = SEE_MASK_DEFAULT|SEE_MASK_INVOKEIDLIST;
	if (hideUI)
	{
		info.fMask = info.fMask|SEE_MASK_FLAG_NO_UI;
	}
	info.hwnd = GetOwnerHWND(window);
	info.lpVerb = operation.wc_str();
	info.lpFile = filePath.wc_str();
	info.lpDirectory = workingFolder.IsEmpty() ? nullptr : workingFolder.wc_str();
	info.lpParameters = arguments.IsEmpty() ? nullptr : arguments.wc_str();
	info.nShow = windowMode;

	::CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED|COINIT_DISABLE_OLE1DDE);
	BOOL status = ::ShellExecuteExW(&info);
	::CoUninitialize();
	return status;
}
bool KxShell::OpenURI(wxWindow* window, const KxURI& uri, bool hideUI)
{
	return Execute(window, uri.BuildUnescapedURI(), {}, {}, {}, SW_SHOWDEFAULT, hideUI);
}

bool KxShell::OpenFolderAndSelectItem(const wxString& filePath)
{
	LPITEMIDLIST item = nullptr;
	SFGAOF attributes = 0;
	if (SUCCEEDED(::SHParseDisplayName(filePath.wc_str(), nullptr, &item, 0, &attributes)))
	{
		::CoInitialize(nullptr);
		HRESULT status = ::SHOpenFolderAndSelectItems(item, 0, nullptr, 0);
		::CoTaskMemFree(item);
		::CoUninitialize();

		return status == S_OK;
	}
	return false;
}
