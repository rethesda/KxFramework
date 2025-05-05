#include "kxf-pch.h"
#include "MouseKeyboardState.h"

#include <Windows.h>
#include <wx/mousestate.h>
#include <wx/kbdstate.h>
#include <wx/utils.h>
#include "kxf/Win32/UndefMacros.h"

namespace
{
	#if !wxUSE_GUI
	bool IsKeyDown(int vk) noexcept
	{
		if (vk == VK_LBUTTON || vk == VK_RBUTTON)
		{
			if (::GetSystemMetrics(SM_SWAPBUTTON))
			{
				if (vk == VK_LBUTTON)
				{
					vk = VK_RBUTTON;
				}
				else
				{
					vk = VK_LBUTTON;
				}
			}
		}

		// The low order bit indicates whether the key was pressed since the last
		// call and the high order one indicates whether it is down right now and
		// we only want that one
		return (::GetAsyncKeyState(vk) & (1 << 15)) != 0;
	}
	bool IsModifierDown(int vk) noexcept
	{
		// GetKeyState() returns different negative values on WinME and WinNT,
		// so simply test for negative value.
		return ::GetKeyState(vk) < 0;
	}
	POINT GetCursorPos() noexcept
	{
		POINT p = {};
		if (!::GetCursorPos(&p))
		{
			DWORD pos = ::GetMessagePos();
			p.x = LOWORD(pos);
			p.y = HIWORD(pos);
		}

		return p;
	}

	wxMouseState wxGetMouseState()
	{
		wxMouseState mouseState;
		auto point = GetCursorPos();

		mouseState.SetX(point.x);
		mouseState.SetY(point.y);
		mouseState.SetLeftDown(IsKeyDown(VK_LBUTTON));
		mouseState.SetMiddleDown(IsKeyDown(VK_MBUTTON));
		mouseState.SetRightDown(IsKeyDown(VK_RBUTTON));
		mouseState.SetAux1Down(IsKeyDown(VK_XBUTTON1));
		mouseState.SetAux2Down(IsKeyDown(VK_XBUTTON2));

		mouseState.SetControlDown(IsModifierDown(VK_CONTROL));
		mouseState.SetShiftDown(IsModifierDown(VK_SHIFT));
		mouseState.SetAltDown(IsModifierDown(VK_MENU));
		mouseState.SetMetaDown(IsModifierDown(VK_LWIN) || IsModifierDown(VK_RWIN));

		return mouseState;
	}
	#endif
}

namespace kxf::wxWidgets
{
	wxMouseState GetMouseState() noexcept
	{
		return wxGetMouseState();
	}
	wxKeyboardState GetKeyboardState() noexcept
	{
		return static_cast<wxKeyboardState>(wxGetMouseState());
	}
}
