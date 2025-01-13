#include "kxf-pch.h"
#include "GDIFont.h"
#include "kxf/Drawing/Font.h"
#include "kxf/wxWidgets/MapDrawing.h"

#include <wx/font.h>
#include <wx/fontutil.h>
#include "kxf/Win32/UndefMacros.h"

namespace
{
	class wxFontRefDataHack final: public wxGDIRefData
	{
		public:
			bool m_sizeUsingPixels = false;
			HFONT m_hFont = nullptr;
			wxNativeFontInfo m_nativeFontInfo;
	};
}

namespace kxf
{
	// GDIFont
	FontEncoding GDIFont::GetDefaultEncoding() noexcept
	{
		return wxWidgets::MapFontEncoding(wxFont::GetDefaultEncoding());
	}
	void GDIFont::SetDefaultEncoding(FontEncoding encoding) noexcept
	{
		wxFont::SetDefaultEncoding(wxWidgets::MapFontEncoding(encoding));
	}
	int GDIFont::GetNumericWeightOf(FontWeight weight) noexcept
	{
		return wxFont::GetNumericWeightOf(wxWidgets::MapFontWeight(weight));
	}

	GDIFont::GDIFont()
	{
		m_Font.ConstructAligned();
	}
	GDIFont::GDIFont(const Font& other)
	{
		m_Font.ConstructAligned(wxWidgets::MapFont(other));
	}
	GDIFont::GDIFont(const GDIFont& other)
	{
		m_Font.ConstructAligned(*other.m_Font);
	}
	GDIFont::GDIFont(const wxFont& other)
	{
		m_Font.ConstructAligned(other);
	}
	GDIFont::GDIFont(const wxNativeFontInfo& other)
	{
		m_Font.ConstructAligned(other);
	}

	GDIFont::GDIFont(float pointSize, FontFamily family, FlagSet<FontStyle> style, FontWeight weight, const String& faceName, FontEncoding encoding)
	{
		m_Font.ConstructAligned(static_cast<int>(pointSize),
								wxWidgets::MapFontFamily(family),
								wxWidgets::MapFontStyle(style),
								wxWidgets::MapFontWeight(weight),
								style.Contains(FontStyle::Underline),
								faceName,
								wxWidgets::MapFontEncoding(encoding)
		);

		m_Font->SetFractionalPointSize(static_cast<double>(pointSize));
		m_Font->SetStrikethrough(style.Contains(FontStyle::Strikethrough));
	}
	GDIFont::GDIFont(const Size& pixelSize, FontFamily family, FlagSet<FontStyle> style, FontWeight weight, const String& faceName, FontEncoding encoding)
	{
		m_Font.ConstructAligned(pixelSize,
								wxWidgets::MapFontFamily(family),
								wxWidgets::MapFontStyle(style),
								wxWidgets::MapFontWeight(weight),
								style.Contains(FontStyle::Underline),
								faceName,
								wxWidgets::MapFontEncoding(encoding)
		);

		m_Font->SetStrikethrough(style.Contains(FontStyle::Strikethrough));
	}

	GDIFont::~GDIFont()
	{
		m_Font.Destroy();
	}

	// IGDIObject
	bool GDIFont::IsNull() const
	{
		return !m_Font.IsConstructed() || !m_Font->IsOk();
	}
	bool GDIFont::IsSameAs(const IGDIObject& other) const
	{
		if (this == &other)
		{
			return true;
		}
		else if (auto ptr = other.QueryInterface<GDIFont>())
		{
			return m_Font->IsSameAs(*ptr->m_Font);
		}
		else
		{
			return m_Font->GetResourceHandle() == other.GetHandle();
		}
	}

	void* GDIFont::GetHandle() const
	{
		return m_Font->GetResourceHandle();
	}
	void* GDIFont::DetachHandle()
	{
		if (wxFontRefDataHack* refData = static_cast<wxFontRefDataHack*>(m_Font->GetRefData()))
		{
			// 'GetResourceHandle' creates the actual font object if it doesn't already exist
			void* handle = m_Font->GetResourceHandle();

			// Clear the internal structures
			refData->m_hFont = nullptr;
			refData->m_nativeFontInfo.lf = {};
			refData->m_sizeUsingPixels = false;

			return handle;
		}
		return nullptr;
	}
	void GDIFont::AttachHandle(void* handle)
	{
		*m_Font = wxFont();

		if (handle)
		{
			m_Font->SetFamily(wxFONTFAMILY_UNKNOWN);
			if (wxFontRefDataHack* refData = static_cast<wxFontRefDataHack*>(m_Font->GetRefData()))
			{
				LOGFONTW fontData = {};
				if (::GetObjectW(handle, sizeof(fontData), &fontData) != 0)
				{
					refData->m_hFont = static_cast<HFONT>(handle);
					refData->m_nativeFontInfo.lf = fontData;
					refData->m_sizeUsingPixels = true;
					return;
				}
			}

			// Delete the handle if we can't attach it
			::DeleteObject(handle);
		}
	}

	// GDIFont
	String GDIFont::GetDescription() const
	{
		return m_Font->GetNativeFontInfoUserDesc();
	}
	GDIFont GDIFont::GetBaseFont() const
	{
		return m_Font->GetBaseFont();
	}

	String GDIFont::Serialize() const
	{
		return m_Font->GetNativeFontInfoDesc();
	}
	bool GDIFont::Deserialize(const String& serializedData)
	{
		return m_Font->SetNativeFontInfo(serializedData);
	}

	String GDIFont::GetFaceName() const
	{
		return m_Font->GetFaceName();
	}
	bool GDIFont::SetFaceName(const String& faceName)
	{
		return m_Font->SetFaceName(faceName);
	}

	float GDIFont::GetPointSize() const
	{
		return static_cast<float>(m_Font->GetFractionalPointSize());
	}
	void GDIFont::SetPointSize(float pointSize)
	{
		m_Font->SetFractionalPointSize(static_cast<double>(pointSize));
	}

	Size GDIFont::GetPixelSize() const
	{
		return Size(m_Font->GetPixelSize());
	}
	void GDIFont::SetPixelSize(const Size& pixelSize)
	{
		m_Font->SetPixelSize(pixelSize);
	}

	void GDIFont::SetSymbolicSize(FontSymbolicSize size)
	{
		m_Font->SetSymbolicSize(wxWidgets::MapFontSymbolicSize(size));
	}
	void GDIFont::SetSymbolicSizeRelativeTo(FontSymbolicSize size, int base)
	{
		m_Font->SetSymbolicSizeRelativeTo(wxWidgets::MapFontSymbolicSize(size), base);
	}
	void GDIFont::ScaleSzie(double scale)
	{
		m_Font->SetFractionalPointSize(m_Font->GetFractionalPointSize() * scale);
	}

	FontEncoding GDIFont::GetEncoding() const
	{
		return wxWidgets::MapFontEncoding(m_Font->GetEncoding());
	}
	void GDIFont::SetEncoding(FontEncoding encoding)
	{
		m_Font->SetEncoding(wxWidgets::MapFontEncoding(encoding));
	}

	FontFamily GDIFont::GetFamily() const
	{
		return wxWidgets::MapFontFamily(m_Font->GetFamily());
	}
	void GDIFont::SetFamily(FontFamily family)
	{
		m_Font->SetFamily(wxWidgets::MapFontFamily(family));
	}

	FlagSet<FontStyle> GDIFont::GetStyle() const
	{
		auto style = wxWidgets::MapFontStyle(m_Font->GetStyle());
		style.Add(FontStyle::Underline, m_Font->GetUnderlined());
		style.Add(FontStyle::Strikethrough, m_Font->GetStrikethrough());

		return style;
	}
	void GDIFont::SetStyle(FlagSet<FontStyle> style)
	{
		m_Font->SetStyle(wxWidgets::MapFontStyle(style));
		m_Font->SetUnderlined(style.Contains(FontStyle::Underline));
		m_Font->SetStrikethrough(style.Contains(FontStyle::Strikethrough));
	}
	void GDIFont::AddStyle(FlagSet<FontStyle> style)
	{
		SetStyle(GetStyle().Add(style));
	}
	void GDIFont::RemoveStyle(FlagSet<FontStyle> style)
	{
		SetStyle(GetStyle().Remove(style));
	}

	kxf::FontWeight GDIFont::GetWeight() const
	{
		return wxWidgets::MapFontWeight(m_Font->GetWeight());
	}
	void GDIFont::SetWeight(FontWeight weight)
	{
		m_Font->SetWeight(wxWidgets::MapFontWeight(weight));
	}
	int GDIFont::GetNumericWeight() const
	{
		return m_Font->GetNumericWeight();
	}
	void GDIFont::SetNumericWeight(int weight)
	{
		m_Font->SetNumericWeight(weight);
	}

	Font GDIFont::ToFont() const
	{
		return *m_Font;
	}

	GDIFont& GDIFont::operator=(const GDIFont& other)
	{
		*m_Font = *other.m_Font;

		return *this;
	}
}
