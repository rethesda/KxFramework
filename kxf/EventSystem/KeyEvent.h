#pragma once
#include "Common.h"
#include "BasicEvent.h"
#include "kxf/Core/UniChar.h"
#include "kxf/Core/KeyCode.h"
#include "kxf/Core/KeyboardState.h"
#include "kxf/Drawing/Geometry.h"

namespace kxf
{
	class KXF_API KeyEvent: public BasicEvent
	{
		public:
			kxf_EVENT_MEMBER(KeyEvent, KeyDown);
			kxf_EVENT_MEMBER(KeyEvent, KeyUp);
			kxf_EVENT_MEMBER(KeyEvent, Char);
			kxf_EVENT_MEMBER(KeyEvent, CharHook);

		private:
			KeyboardState m_KeyboardState;
			Point m_Position = Point::UnspecifiedPosition();
			KeyCode m_KeyCode = KeyCode::None;
			UniChar m_UnicodeKey;

		public:
			KeyEvent() = default;
			KeyEvent(KeyCode keyCode, KeyboardState keyboardState = {}) noexcept
				:m_KeyboardState(std::move(keyboardState)), m_KeyCode(keyCode)
			{
				if (ToInt(keyCode) >= 33 && ToInt(keyCode) <= 126)
				{
					m_UnicodeKey = ToInt(keyCode);
				}
			}
			KeyEvent(KeyCode keyCode, UniChar unicodeKey, KeyboardState keyboardState = {}) noexcept
				:m_KeyboardState(std::move(keyboardState)), m_KeyCode(keyCode), m_UnicodeKey(unicodeKey)
			{
			}

		public:
			std::unique_ptr<IEvent> Move() noexcept override
			{
				return std::make_unique<KeyEvent>(std::move(*this));
			}

			KeyboardState GetKeyboardState() const noexcept
			{
				return m_KeyboardState;
			}
			KeyCode GetKeyCode() const noexcept
			{
				return m_KeyCode;
			}
			UniChar GetUnicodeKey() const noexcept
			{
				return m_UnicodeKey;
			}

			Point GetPosition() const noexcept
			{
				return m_Position;
			}
			void SetPosition(const Point& pos) noexcept
			{
				m_Position = pos;
			}
	};
}
