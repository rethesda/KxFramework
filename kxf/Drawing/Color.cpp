#include "kxf-pch.h"
#include "Color.h"
#include "kxf/Core/RegEx.h"
#include "kxf/Core/String.h"
#include "kxf/Utility/Common.h"
#include <wx/colour.h>
#include <wx/brush.h>
#include <wx/pen.h>

namespace
{
	kxf::String CSS2RGB(const kxf::Color& color, kxf::C2SAlpha alpha)
	{
		using namespace kxf;

		switch (alpha)
		{
			case C2SAlpha::Never:
			{
				const auto fixed = color.GetFixed8();
				return Format("rgb({}, {}, {})", fixed.Red, fixed.Green, fixed.Blue);
			}
			case C2SAlpha::Always:
			{
				const auto fixed = color.GetFixed8();
				const auto normalized = color.GetNormalized();
				return Format("rgba({}, {}, {}, {})", fixed.Red, fixed.Green, fixed.Blue, normalized.Alpha);
			}
			case C2SAlpha::Auto:
			{
				return color.IsOpaque() ? CSS2RGB(color, C2SAlpha::Never) : CSS2RGB(color, C2SAlpha::Always);
			}
		};
		return {};
	}
	kxf::String CSS2HSL(const kxf::Color& color, kxf::C2SAlpha alpha)
	{
		using namespace kxf;

		switch (alpha)
		{
			case C2SAlpha::Never:
			{
				const auto hsl = color.GetHSL();
				return Format("hsl({}, {}, {})", hsl.Hue.ToDegrees(), hsl.Saturation, hsl.Lightness);
			}
			case C2SAlpha::Always:
			{
				const auto hsl = color.GetHSL();
				const auto normalized = color.GetNormalized();
				return Format("hsla({}, {}, {}, {})", hsl.Hue.ToDegrees(), hsl.Saturation, hsl.Lightness, normalized.Alpha);
			}
			case C2SAlpha::Auto:
			{
				return color.IsOpaque() ? CSS2HSL(color, C2SAlpha::Never) : CSS2HSL(color, C2SAlpha::Always);
			}
		};
		return {};
	}

	kxf::String HTML2RGB(const kxf::Color& color, kxf::C2SAlpha alpha)
	{
		using namespace kxf;

		switch (alpha)
		{
			case C2SAlpha::Never:
			{
				const auto fixed = color.GetFixed8();
				return Format("#{:0>2x}{:0>2x}{:0>2x}", fixed.Red, fixed.Green, fixed.Blue);
			}
			case C2SAlpha::Always:
			{
				const auto fixed = color.GetFixed8();
				return Format("#{:0>2x}{:0>2x}{:0>2x}{:0>2x}", fixed.Red, fixed.Green, fixed.Blue, fixed.Alpha);
			}
			case C2SAlpha::Auto:
			{
				return color.IsOpaque() ? HTML2RGB(color, C2SAlpha::Never) : HTML2RGB(color, C2SAlpha::Always);
			}
		};
		return {};
	}
	kxf::String HTML2HSL(const kxf::Color& color, kxf::C2SAlpha alpha)
	{
		using namespace kxf;

		switch (alpha)
		{
			case C2SAlpha::Never:
			{
				const auto hsl = color.GetHSL();
				return Format("hsl({}, {}%, {}%)", static_cast<int>(hsl.Hue.ToDegrees()), static_cast<int>(hsl.Saturation * 100), static_cast<int>(hsl.Lightness * 100));
			}
			case C2SAlpha::Always:
			{
				const auto hsl = color.GetHSL();
				return Format("hsl({}, {}%, {}%, {})", static_cast<int>(hsl.Hue.ToDegrees()), static_cast<int>(hsl.Saturation * 100), static_cast<int>(hsl.Lightness * 100), hsl.Alpha);
			}
			case C2SAlpha::Auto:
			{
				return color.IsOpaque() ? HTML2HSL(color, C2SAlpha::Never) : HTML2HSL(color, C2SAlpha::Always);
			}
		};
		return {};
	}
}
namespace
{
	class KnownColorInfo final
	{
		private:
			kxf::StringView m_Name;
			kxf::Color m_Color;

		public:
			constexpr KnownColorInfo(kxf::StringView name, const kxf::PackedRGB<uint8_t>& rgb) noexcept
				:m_Name(name), m_Color(rgb)
			{
			}

		public:
			constexpr kxf::StringView GetName() const noexcept
			{
				return m_Name;
			}
			constexpr kxf::Color GetColor() const noexcept
			{
				return m_Color;
			}
	};

	constexpr KnownColorInfo g_KnownColors[] =
	{
		{kxfS("AQUAMARINE"), {112, 219, 147}},
		{kxfS("BLACK"), {0, 0, 0}},
		{kxfS("BLUE"), {0, 0, 255}},
		{kxfS("BLUE VIOLET"), {159, 95, 159}},
		{kxfS("BROWN"), {165, 42, 42}},
		{kxfS("CADET BLUE"), {95, 159, 159}},
		{kxfS("CORAL"), {255, 127, 0}},
		{kxfS("CORNFLOWER BLUE"), {66, 66, 111}},
		{kxfS("CYAN"), {0, 255, 255}},
		{kxfS("DARK GRAY"), {47, 47, 47}},

		{kxfS("DARK GREEN"), {47, 79, 47}},
		{kxfS("DARK OLIVE GREEN"), {79, 79, 47}},
		{kxfS("DARK ORCHID"), {153, 50, 204}},
		{kxfS("DARK SLATE BLUE"), {107, 35, 142}},
		{kxfS("DARK SLATE GRAY"), {47, 79, 79}},
		{kxfS("DARK TURQUOISE"), {112, 147, 219}},
		{kxfS("DIM GRAY"), {84, 84, 84}},
		{kxfS("FIREBRICK"), {142, 35, 35}},
		{kxfS("FOREST GREEN"), {35, 142, 35}},
		{kxfS("GOLD"), {204, 127, 50}},
		{kxfS("GOLDENROD"), {219, 219, 112}},
		{kxfS("GRAY"), {128, 128, 128}},
		{kxfS("GREEN"), {0, 255, 0}},
		{kxfS("GREEN YELLOW"), {147, 219, 112}},
		{kxfS("INDIAN RED"), {79, 47, 47}},
		{kxfS("KHAKI"), {159, 159, 95}},
		{kxfS("LIGHT BLUE"), {191, 216, 216}},
		{kxfS("LIGHT GRAY"), {192, 192, 192}},
		{kxfS("LIGHT STEEL BLUE"), {143, 143, 188}},
		{kxfS("LIME GREEN"), {50, 204, 50}},
		{kxfS("LIGHT MAGENTA"), {255, 119, 255}},
		{kxfS("MAGENTA"), {255, 0, 255}},
		{kxfS("MAROON"), {142, 35, 107}},
		{kxfS("MEDIUM AQUAMARINE"), {50, 204, 153}},
		{kxfS("MEDIUM GRAY"), {100, 100, 100}},
		{kxfS("MEDIUM BLUE"), {50, 50, 204}},
		{kxfS("MEDIUM FOREST GREEN"), {107, 142, 35}},
		{kxfS("MEDIUM GOLDENROD"), {234, 234, 173}},
		{kxfS("MEDIUM ORCHID"), {147, 112, 219}},
		{kxfS("MEDIUM SEA GREEN"), {66, 111, 66}},
		{kxfS("MEDIUM SLATE BLUE"), {127, 0, 255}},
		{kxfS("MEDIUM SPRING GREEN"), {127, 255, 0}},
		{kxfS("MEDIUM TURQUOISE"), {112, 219, 219}},
		{kxfS("MEDIUM VIOLET RED"), {219, 112, 147}},
		{kxfS("MIDNIGHT BLUE"), {47, 47, 79}},
		{kxfS("NAVY"), {35, 35, 142}},
		{kxfS("ORANGE"), {204, 50, 50}},
		{kxfS("ORANGE RED"), {255, 0, 127}},
		{kxfS("ORCHID"), {219, 112, 219}},
		{kxfS("PALE GREEN"), {143, 188, 143}},
		{kxfS("PINK"), {255, 192, 203}},
		{kxfS("PLUM"), {234, 173, 234}},
		{kxfS("PURPLE"), {176, 0, 255}},
		{kxfS("RED"), {255, 0, 0}},
		{kxfS("SALMON"), {111, 66, 66}},
		{kxfS("SEA GREEN"), {35, 142, 107}},
		{kxfS("SIENNA"), {142, 107, 35}},
		{kxfS("SKY BLUE"), {50, 153, 204}},
		{kxfS("SLATE BLUE"), {0, 127, 255}},
		{kxfS("SPRING GREEN"), {0, 255, 127}},
		{kxfS("STEEL BLUE"), {35, 107, 142}},
		{kxfS("TAN"), {219, 147, 112}},
		{kxfS("THISTLE"), {216, 191, 216}},
		{kxfS("TURQUOISE"), {173, 234, 234}},
		{kxfS("VIOLET"), {79, 47, 79}},
		{kxfS("VIOLET RED"), {204, 50, 153}},
		{kxfS("WHEAT"), {216, 216, 191}},
		{kxfS("WHITE"), {255, 255, 255}},
		{kxfS("YELLOW"), {255, 255, 0}},
		{kxfS("YELLOW GREEN"), {153, 204, 50}}
	};
}

namespace kxf
{
	Color Color::FromString(const String& value, ColorSpace* colorSpace)
	{
		Utility::SetIfNotNull(colorSpace, ColorSpace::None);

		if (!value.IsEmpty())
		{
			if (value.front() == '#')
			{
				auto r = value.SubMid(1, 2).ParseInteger<uint8_t>(16);
				auto g = value.SubMid(3, 2).ParseInteger<uint8_t>(16);
				auto b = value.SubMid(5, 2).ParseInteger<uint8_t>(16);
				if (r && g && b)
				{
					Utility::SetIfNotNull(colorSpace, ColorSpace::RGB);

					auto a = value.SubMid(7, 2).ParseInteger<uint8_t>(16);
					return FromFixed8(*r, *g, *b, a.value_or(255));
				}
			}
			else if (RegEx regEx(wxS(R"(rgba?\((\d+),\s+(\d+),\s+(\d+),?\s*([\d\.]*)\))")); regEx.Matches(value))
			{
				auto r = regEx.GetMatch(value, 1).ParseInteger<uint8_t>();
				auto g = regEx.GetMatch(value, 2).ParseInteger<uint8_t>();
				auto b = regEx.GetMatch(value, 3).ParseInteger<uint8_t>();

				if (r && g && b)
				{
					Utility::SetIfNotNull(colorSpace, ColorSpace::RGB);

					auto a = regEx.GetMatch(value, 4).ParseFloatingPoint<float>();
					return FromFixed8(*r, *g, *b, a.value_or(1) * 255);
				}
			}
			else if (RegEx regEx(wxS(R"(hsla?\(([\d\.]+),\s+([\d\.]+),\s+([\d\.]+),?\s*([\d\.]*)\))")); regEx.Matches(value))
			{
				auto h = regEx.GetMatch(value, 1).ParseFloatingPoint<float>();
				auto s = regEx.GetMatch(value, 2).ParseFloatingPoint<float>();
				auto l = regEx.GetMatch(value, 3).ParseFloatingPoint<float>();

				if (h && s && l)
				{
					Utility::SetIfNotNull(colorSpace, ColorSpace::HSL);

					auto a = regEx.GetMatch(value, 4).ParseFloatingPoint<float>();
					return FromHSL(PackedHSL{Angle::FromDegrees(*h), *s, *l, a.value_or(1)});
				}
			}
		}
		return {};
	}
	Color Color::FromColorName(const String& name)
	{
		auto it = std::find_if(std::begin(g_KnownColors), std::end(g_KnownColors), [&](const KnownColorInfo& colorInfo)
		{
			return name.IsSameAs(colorInfo.GetName(), StringActionFlag::IgnoreCase);
		});
		if (it != std::end(g_KnownColors))
		{
			return it->GetColor();
		}

		#if wxUSE_GUI
		return wxTheColourDatabase->Find(name);
		#else
		return {};
		#endif
	}

	Color::Color(const wxColour& other) noexcept
	{
		if (other.IsOk())
		{
			m_Value = ToNormalizedBBP(other.Red(), other.Green(), other.Blue(), other.Alpha());
		}
	}

	String Color::ToString(C2SFormat format, C2SAlpha alpha, ColorSpace colorSpace) const
	{
		switch (format)
		{
			case C2SFormat::CSS:
			{
				switch (colorSpace)
				{
					case ColorSpace::RGB:
					{
						return CSS2RGB(*this, alpha);
					}
					case ColorSpace::HSL:
					{
						return CSS2HSL(*this, alpha);
					}
				};
				break;
			}
			case C2SFormat::HTML:
			{
				switch (colorSpace)
				{
					case ColorSpace::RGB:
					{
						return HTML2RGB(*this, alpha);
					}
					case ColorSpace::HSL:
					{
						return HTML2HSL(*this, alpha);
					}
				};
				break;
			}
		};
		return {};
	}
	String Color::GetColorName() const
	{
		const auto fixed8 = GetFixed8();
		auto it = std::find_if(std::begin(g_KnownColors), std::end(g_KnownColors), [&](const KnownColorInfo& colorInfo)
		{
			return colorInfo.GetColor().GetFixed8() == fixed8;
		});
		if (it != std::end(g_KnownColors))
		{
			return it->GetName();
		}

		#if wxUSE_GUI
		return wxTheColourDatabase->FindName(wxColour(fixed8.Red, fixed8.Green, fixed8.Blue, wxALPHA_OPAQUE));
		#else
		return {};
		#endif
	}

	std::strong_ordering Color::operator<=>(const wxColour& other) const noexcept
	{
		return GetFixed8() <=> PackedRGBA<uint8_t>(other.Red(), other.Green(), other.Blue(), other.Alpha());
	}
	bool Color::operator==(const wxColour& other) const noexcept
	{
		if (IsValid() && other.IsOk())
		{
			return GetFixed8() == PackedRGBA<uint8_t>(other.Red(), other.Green(), other.Blue(), other.Alpha());
		}
		return false;
	}

	Color::operator wxColour() const noexcept
	{
		if (IsValid())
		{
			auto temp = GetFixed8();
			return wxColour(temp.Red, temp.Green, temp.Blue, temp.Alpha);
		}
		return {};
	}
	Color::operator wxBrush() const noexcept
	{
		return static_cast<wxColour>(*this);
	}
	Color::operator wxPen() const noexcept
	{
		return static_cast<wxColour>(*this);
	}
}
