#pragma once
#include "Common.h"
#include "kxf/Core/UninitializedStorage.h"
#include "kxf/Drawing/IImage2D.h"
#include "kxf/Drawing/IGDIObject.h"
class wxIcon;

namespace kxf
{
	class GDIBitmap;
	class GDICursor;
}

namespace kxf
{
	class KXF_API GDIIcon final: public RTTI::DynamicImplementation<GDIIcon, IGDIObject, IImage2D>
	{
		kxf_RTTI_DeclareIID(GDIIcon, {0x5da6784, 0xf20c, 0x4ad6, {0xb1, 0x3b, 0x3a, 0xa1, 0x2d, 0xd9, 0x66, 0x4a}});

		private:
			UninitializedStorage<wxIcon, 32, 0> m_Icon;

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

		public:
			GDIIcon();
			GDIIcon(const wxIcon& other);

			GDIIcon(const GDIIcon& other);
			GDIIcon(const GDIBitmap& other);
			GDIIcon(const GDICursor& other);
			GDIIcon(const BitmapImage& other);
			GDIIcon(const char* xbm, const Size& size);

			~GDIIcon();

		public:
			// IGDIObject
			bool IsNull() const override;
			bool IsSameAs(const IGDIObject& other) const override;
			std::shared_ptr<IGDIObject> CloneGDIObject() const override
			{
				return std::make_shared<GDIIcon>(*m_Icon);
			}

			void* GetHandle() const override;
			void* DetachHandle() override;
			void AttachHandle(void* handle) override;

			// IImage2D
			bool IsSameAs(const IImage2D& other) const override;
			std::shared_ptr<IImage2D> CloneImage2D() const override
			{
				return std::make_shared<GDIIcon>(*m_Icon);
			}

			Size GetSize() const override;
			ColorDepth GetColorDepth() const override;
			UniversallyUniqueID GetFormat() const override;

			bool Create(const Size& size) override;
			bool Load(IInputStream& stream, const UniversallyUniqueID& format = ImageFormat::Any, size_t index = npos);
			bool Save(IOutputStream& stream, const UniversallyUniqueID& format) const;

			BitmapImage ToBitmapImage(const Size& size = Size::UnspecifiedSize(), InterpolationQuality interpolationQuality = InterpolationQuality::Default) const override;

			// GDIIcon
			wxIcon& AsWXIcon() noexcept
			{
				return *m_Icon;
			}
			const wxIcon& AsWXIcon() const noexcept
			{
				return *m_Icon;
			}

			GDICursor ToGDICursor(const Point& hotSpot = Point::UnspecifiedPosition()) const;
			GDIBitmap ToGDIBitmap(const Size& size = Size::UnspecifiedSize(), InterpolationQuality interpolationQuality = InterpolationQuality::Default) const;

			GDIIcon ConvertToDisabled(Angle brightness = Angle::FromNormalized(1)) const;

		public:
			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}

			GDIIcon& operator=(const GDIIcon& other);
	};
}
