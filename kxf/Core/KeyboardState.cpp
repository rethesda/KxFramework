#include "kxf-pch.h"
#include "KeyboardState.h"
#include "kxf/wxWidgets/MouseKeyboardState.h"
#include <wx/mousestate.h>

namespace kxf
{
	KeyboardState KeyboardState::GetCurrent() noexcept
	{
		return wxWidgets::GetKeyboardState();
	}

	KeyboardState::KeyboardState(const wxKeyboardState& state) noexcept
	{
		m_Modifiers.Add(KeyModifier::Alt, state.AltDown());
		m_Modifiers.Add(KeyModifier::Ctrl, state.ControlDown());
		m_Modifiers.Add(KeyModifier::Shift, state.ShiftDown());
		m_Modifiers.Add(KeyModifier::Meta, state.MetaDown());
	}
}
