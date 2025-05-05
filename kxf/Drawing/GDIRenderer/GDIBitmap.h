#pragma once
#include "Common.h"
#include "kxf/Core/UninitializedStorage.h"
#include "kxf/Drawing/IImage2D.h"
#include "kxf/Drawing/IGDIObject.h"
class wxDC;
class wxBitmap;

namespace kxf
{
	class GDIContext;
	class GDICursor;
	class GDIIcon;
}

namespace kxf
{
	class KXF_API GDIBitmap final: public RTTI::DynamicImplementation<GDIBitmap, IGDIObject, IImage2D>
	{
		kxf_RTTI_DeclareIID(GDIBitmap, {0xd4c3e7be, 0xf0fd, 0x4c38, {0xa1, 0x94, 0x16, 0xb3, 0x9, 0xa, 0x34, 0xb5}});

		private:
			UninitializedStorage<wxBitmap, 32, 0> m_Bitmap;

		private:
			std::optional<String> GetOption(const String& name) const override
			{
				return {};
			}
			std::optional<int> GetOptionInt(const String& name) const override
			{
				return {};
			}

			void SetOption(const String& name, int value) override
			{
			}
			void SetOption(const String& name, const String& value) override
			{
			}

		private:
			void Initialize();

		public:
			GDIBitmap();
			GDIBitmap(const wxBitmap& other);

			GDIBitmap(const GDIIcon& other);
			GDIBitmap(const GDICursor& other);
			GDIBitmap(const GDIBitmap& other);
			GDIBitmap(const BitmapImage& other);
			GDIBitmap(const BitmapImage& other, const wxDC& dc);

			GDIBitmap(const Size& size, ColorDepth depth = {});
			GDIBitmap(const Size& size, const wxDC& dc);
			GDIBitmap(const char* xbm, const Size& size, ColorDepth depth = ColorDepthDB::BPP1);

			~GDIBitmap();

		public:
			// IGDIObject
			bool IsNull() const override;
			bool IsSameAs(const IGDIObject& other) const override;
			std::shared_ptr<IGDIObject> CloneGDIObject() const override
			{
				return std::make_shared<GDIBitmap>(*m_Bitmap);
			}

			void* GetHandle() const override;
			void* DetachHandle() override;
			void AttachHandle(void* handle) override;

			// IImage2D
			bool IsSameAs(const IImage2D& other) const override;
			std::shared_ptr<IImage2D> CloneImage2D() const override
			{
				return std::make_shared<GDIBitmap>(*m_Bitmap);
			}

			Size GetSize() const override;
			ColorDepth GetColorDepth() const override;
			UniversallyUniqueID GetFormat() const override;

			bool Create(const Size& size) override;
			bool Load(IInputStream& stream, const UniversallyUniqueID& format = ImageFormat::Any, size_t index = npos);
			bool Save(IOutputStream& stream, const UniversallyUniqueID& format) const;

			BitmapImage ToBitmapImage(const Size& size = Size::UnspecifiedSize(), InterpolationQuality interpolationQuality = InterpolationQuality::Default) const override;

			// GDIBitmap
			wxBitmap& AsWXBitmap() noexcept
			{
				return *m_Bitmap;
			}
			const wxBitmap& AsWXBitmap() const noexcept
			{
				return *m_Bitmap;
			}

			GDICursor ToGDICursor(const Point& hotSpot = Point::UnspecifiedPosition()) const;
			GDIIcon ToGDIIcon() const;

			GDIBitmap GetSubBitmap(const Rect& rect) const;
			GDIBitmap ConvertToDisabled(Angle brightness = Angle::FromNormalized(1)) const;
			GDIBitmap GetScaled(const Size& size = Size::UnspecifiedSize(), InterpolationQuality interpolationQuality = InterpolationQuality::Default) const;

			void ForceAlpha();
			void UpdateAlpha();

		public:
			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}

			GDIBitmap& operator=(const GDIBitmap& other);
	};
}
