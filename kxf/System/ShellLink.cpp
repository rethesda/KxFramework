#include "kxf-pch.h"
#include "ShellLink.h"
#include "Private/Shell.h"
#include "kxf/EventSystem/KeyEvent.h"
#include "kxf/Utility/Memory.h"

#include <wx/event.h>
#include <ShObjIDL.h>
#include "kxf/Win32/UndefMacros.h"

namespace kxf
{
	ShellLink::ShellLink() noexcept
	{
		COMPtr<IShellLinkW> shellLink;
		if (COM::CreateInstance(CLSID_ShellLink, COMClassContext::InprocServer, &shellLink))
		{
			m_ShellLink = std::move(shellLink);
		}
	}
	ShellLink::~ShellLink() noexcept
	{
	}

	HResult ShellLink::Load(const FSPath& path)
	{
		HResult hr = E_INVALIDARG;

		if (path)
		{
			COMPtr<IPersistFile> persistFile;
			if (hr = m_ShellLink->QueryInterface(&persistFile))
			{
				String pathString = path.GetFullPath();
				return persistFile->Load(pathString.wc_str(), 0);
			}
		}
		return hr;
	}
	HResult ShellLink::Save(const FSPath& path) const
	{
		HResult hr = E_INVALIDARG;

		if (path)
		{
			COMPtr<IPersistFile> persistFile;
			if (hr = m_ShellLink->QueryInterface(&persistFile))
			{
				String pathString = path.GetFullPath();
				return persistFile->Save(pathString.wc_str(), TRUE);
			}
		}
		return hr;
	}

	FSPath ShellLink::GetTarget() const
	{
		XChar buffer[INT16_MAX] = {};
		WIN32_FIND_DATAW findData = {};
		if (HResult(m_ShellLink->GetPath(buffer, std::size(buffer), &findData, 0)))
		{
			return buffer;
		}
		return {};
	}
	HResult ShellLink::SetTarget(const FSPath& path)
	{
		String pathString = path.GetFullPath();
		return m_ShellLink->SetPath(pathString.wc_str());
	}

	String ShellLink::GetArguments() const
	{
		XChar buffer[INT16_MAX] = {};
		if (HResult(m_ShellLink->GetArguments(buffer, std::size(buffer))))
		{
			return buffer;
		}
		return {};
	}
	HResult ShellLink::SetArguments(const String& value)
	{
		return m_ShellLink->SetArguments(value.wc_str());
	}

	FSPath ShellLink::GetWorkingDirectory() const
	{
		XChar buffer[INT16_MAX] = {};
		if (HResult(m_ShellLink->GetWorkingDirectory(buffer, std::size(buffer))))
		{
			return buffer;
		}
		return {};
	}
	HResult ShellLink::SetWorkingDirectory(const FSPath& path)
	{
		String pathString = path.GetFullPath();
		return m_ShellLink->SetWorkingDirectory(pathString.wc_str());
	}

	String ShellLink::GetDescription() const
	{
		XChar buffer[INFOTIPSIZE] = {};
		if (HResult(m_ShellLink->GetDescription(buffer, std::size(buffer))))
		{
			return buffer;
		}
		return {};
	}
	HResult ShellLink::SetDescription(const String& value)
	{
		return m_ShellLink->SetDescription(value.wc_str());
	}

	FSPath ShellLink::GetIconLocation() const
	{
		int index = 0;
		if (HResult(m_ShellLink->GetIconLocation(nullptr, 0, &index)))
		{
			wchar_t path[std::numeric_limits<int16_t>::max()] = {};
			if (HResult(m_ShellLink->GetIconLocation(path, std::size(path), &index)))
			{
				return path;
			}
		}
		return {};
	}
	HResult ShellLink::SetIconLocation(const FSPath& path, int index)
	{
		String pathString = path.GetFullPath();
		return m_ShellLink->SetIconLocation(pathString.wc_str(), index);
	}
	std::optional<int> ShellLink::GetIconIndex() const noexcept
	{
		int index = -1;
		if (HResult(m_ShellLink->GetIconLocation(nullptr, 0, &index)))
		{
			return index;
		}
		return {};
	}

	FlagSet<SHWindowCommand> ShellLink::GetShowCommand() const noexcept
	{
		int command = 0;
		if (HResult(m_ShellLink->GetShowCmd(&command)))
		{
			return Shell::Private::MapSHWindowCommand(command);
		}
		return {};
	}
	HResult ShellLink::SetShowCommand(FlagSet<SHWindowCommand> command) noexcept
	{
		return m_ShellLink->SetShowCmd(Shell::Private::MapSHWindowCommand(command).value_or(SW_SHOWNORMAL));
	}

	KeyEvent ShellLink::GetHotKey() const noexcept
	{
		KeyEvent keyState;

		WORD hotKeys = 0;
		if (HResult(m_ShellLink->GetHotkey(&hotKeys)))
		{
			Utility::CompositeInteger<BYTE> hotKeyData(hotKeys);
			auto keyCode = hotKeyData.GetLow();
			auto modifiers = hotKeyData.GetHigh();

			FlagSet<KeyModifier> keyModifiers;
			keyModifiers.Add(KeyModifier::Ctrl, modifiers & HOTKEYF_CONTROL);
			keyModifiers.Add(KeyModifier::Alt, modifiers & HOTKEYF_ALT);
			keyModifiers.Add(KeyModifier::Shift, modifiers & HOTKEYF_SHIFT);

			return KeyEvent(FromInt<KeyCode>(keyCode), KeyboardState(keyModifiers));
		}
		return {};
	}
	HResult ShellLink::SetHotKey(const KeyEvent& keyState) noexcept
	{
		auto keyboardState = keyState.GetKeyboardState();

		FlagSet<BYTE> modifiers;
		modifiers.Add(HOTKEYF_CONTROL, keyboardState.IsCtrlDown());
		modifiers.Add(HOTKEYF_ALT, keyboardState.IsAltDown());
		modifiers.Add(HOTKEYF_SHIFT, keyboardState.IsShiftDown());

		return m_ShellLink->SetHotkey(*Utility::CompositeInteger(*modifiers, ToInt<BYTE>(keyState.GetKeyCode())));
	}
}
