#pragma once
#include "Common.h"
#include "Kx/General/OptionSet.h"

namespace KxFramework::UI::DataView
{
	class Renderer;
	class RenderEngine;
}

namespace KxFramework::UI::DataView 
{
	enum class CellOption
	{
		None = 0,
		Enabled = 1 << 0,
		Editable = 1 << 1,
		HighlightItem = 1 << 2,
		ShowAccelerators = 1 << 3,

		Default = Enabled|Editable
	};
	enum class CellBGOption
	{
		None = 0,
		Header = 1 << 0,
		Button = 1 << 1,
		ComboBox = 1 << 2,

		Default = None
	};
	enum class CellFontOption
	{
		None = 0,
		Bold = 1 << 0,
		Italic = 1 << 1,
		Underlined = 1 << 2,
		Strikethrough = 1 << 3,

		Default = None
	};
}
namespace KxFramework::EnumClass
{
	Kx_EnumClass_AllowEverything(UI::DataView::CellOption);
	Kx_EnumClass_AllowEverything(UI::DataView::CellBGOption);
	Kx_EnumClass_AllowEverything(UI::DataView::CellFontOption);
}

namespace KxFramework::UI::DataView::CellAttributes
{
	class Options final: public OptionSet<CellOption, CellOption::Default>
	{
		friend class Renderer;
		friend class RenderEngine;

		private:
			Color m_ForegroundColor;
			Color m_BackgroundColor;
			wxAlignment m_Alignment = wxALIGN_INVALID;

		private:
			bool NeedDCAlteration() const
			{
				return HasColors();
			}
			void ChangeAlpha(Color& color, uint8_t alpha) const
			{
				color.SetAlpha8(alpha);
			}

		public:
			bool IsDefault() const
			{
				return !HasColors() && !HasAlignment() && RawGetValue() == CellOption::Default;
			}

			// Colors
			bool HasColors() const
			{
				return HasBackgroundColor() || HasForegroundColor();
			}

			bool HasBackgroundColor() const
			{
				return m_BackgroundColor.IsValid();
			}
			Color GetBackgroundColor() const
			{
				return m_BackgroundColor;
			}
			void SetBackgroundColor(const Color& color)
			{
				m_BackgroundColor = color;
			}
			void SetBackgroundColor(const Color& color, uint8_t alpha)
			{
				m_BackgroundColor = color;
				ChangeAlpha(m_BackgroundColor, alpha);
			}

			bool HasForegroundColor() const
			{
				return m_ForegroundColor.IsValid();
			}
			Color GetForegroundColor() const
			{
				return m_ForegroundColor;
			}
			void SetForegroundColor(const Color& color)
			{
				m_ForegroundColor = color;
			}
			void SetForegroundColor(const Color& color, uint8_t alpha)
			{
				m_ForegroundColor = color;
				ChangeAlpha(m_ForegroundColor, alpha);
			}
			
			// Alignment
			bool HasAlignment() const
			{
				return m_Alignment != wxALIGN_INVALID;
			}
			wxAlignment GetAlignment() const
			{
				return m_Alignment;
			}
			void SetAlignment(wxAlignment alignment)
			{
				m_Alignment = alignment;
			}
			void SetAlignment(int alignment)
			{
				m_Alignment = static_cast<wxAlignment>(alignment);
			}
	};
}

namespace KxFramework::UI::DataView::CellAttributes
{
	class FontOptions final: public OptionSet<CellFontOption, CellFontOption::Default>
	{
		friend class Renderer;
		friend class RenderEngine;

		private:
			wxString m_FontFace;
			int32_t m_FontSize = 0;

		private:
			bool NeedDCAlteration() const
			{
				return HasFontFace() || HasFontSize() || RawGetValue() != CellFontOption::Default;
			}

		public:
			bool IsDefault() const
			{
				return !NeedDCAlteration();
			}

			bool HasFontFace() const
			{
				return !m_FontFace.IsEmpty();
			}
			const wxString& GetFontFace() const
			{
				return m_FontFace;
			}
			void SetFontFace(const wxString& faceName)
			{
				m_FontFace = faceName;
			}

			bool HasFontSize() const
			{
				return m_FontSize > 0;
			}
			int32_t GetFontSize() const
			{
				return m_FontSize;
			}
			void SetFontSize(int32_t value)
			{
				m_FontSize = std::clamp(value, 0, std::numeric_limits<int32_t>::max());
			}
	};
}

namespace KxFramework::UI::DataView::CellAttributes
{
	class BGOptions final: public OptionSet<CellBGOption, CellBGOption::Default>
	{
		friend class Renderer;
		friend class RenderEngine;

		private:
			bool NeedDCAlteration() const
			{
				return RawGetValue() != CellBGOption::Default;
			}

		public:
			bool IsDefault() const
			{
				return !NeedDCAlteration();
			}
	};
}