#pragma once
#include "Common.h"
#include "kxf/Drawing/Geometry.h"
#include "kxf/Drawing/IGDIObject.h"
#include "kxf/Core/UninitializedStorage.h"
class wxFont;
class wxNativeFontInfo;

namespace kxf
{
	class Font;
}

namespace kxf
{
	class KXF_API GDIFont: public RTTI::ExtendInterface<GDIFont, IGDIObject>
	{
		kxf_RTTI_DeclareIID(GDIFont, {0xbeb3a65c, 0xf639, 0x4e44, {0x80, 0x3a, 0x1b, 0x53, 0xf6, 0x9c, 0x61, 0xd8}});

		public:
			static FontEncoding GetDefaultEncoding() noexcept;
			static void SetDefaultEncoding(FontEncoding encoding) noexcept;

			static int GetNumericWeightOf(FontWeight weight) noexcept;

		private:
			UninitializedStorage<wxFont, 32, 0> m_Font;

		public:
			GDIFont();
			GDIFont(const Font& other);
			GDIFont(const GDIFont& other);
			GDIFont(const wxFont& other);
			GDIFont(const wxNativeFontInfo& other);

			GDIFont(float pointSize, FontFamily family, FlagSet<FontStyle> style, FontWeight weight, const String& faceName = {}, FontEncoding encoding = FontEncoding::Default);
			GDIFont(const Size& pixelSize, FontFamily family, FlagSet<FontStyle> style, FontWeight weight, const String& faceName = {}, FontEncoding encoding = FontEncoding::Default);

			~GDIFont();

		public:
			// IGDIObject
			bool IsNull() const override;
			bool IsSameAs(const IGDIObject& other) const override;
			std::shared_ptr<IGDIObject> CloneGDIObject() const override
			{
				return std::make_shared<GDIFont>(*m_Font);
			}

			void* GetHandle() const override;
			void* DetachHandle() override;
			void AttachHandle(void* handle) override;

			// GDIFont
			String GetDescription() const;
			GDIFont GetBaseFont() const;

			String Serialize() const;
			bool Deserialize(const String& serializedData);

			String GetFaceName() const;
			bool SetFaceName(const String& faceName);

			float GetPointSize() const;
			void SetPointSize(float pointSize);

			Size GetPixelSize() const;
			void SetPixelSize(const Size& pixelSize);

			void SetSymbolicSize(FontSymbolicSize size);
			void SetSymbolicSizeRelativeTo(FontSymbolicSize size, int base);
			void ScaleSzie(double scale);

			FontEncoding GetEncoding() const;
			void SetEncoding(FontEncoding encoding);

			FontFamily GetFamily() const;
			void SetFamily(FontFamily family);

			FlagSet<FontStyle> GetStyle() const;
			void SetStyle(FlagSet<FontStyle> style);
			void AddStyle(FlagSet<FontStyle> style);
			void RemoveStyle(FlagSet<FontStyle> style);

			FontWeight GetWeight() const;
			void SetWeight(FontWeight weight);
			int GetNumericWeight() const;
			void SetNumericWeight(int weight);

			Font ToFont() const;
			wxFont& AsWXFont() noexcept
			{
				return *m_Font;
			}
			const wxFont& AsWXFont() const noexcept
			{
				return *m_Font;
			}

		public:
			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}

			GDIFont& operator=(const GDIFont& other);
	};
}
