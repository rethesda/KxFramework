#pragma once
#include "Common.h"
#include "kxf/Core/UninitializedStorage.h"
#include "kxf/Drawing/IImage2D.h"
#include "kxf/Drawing/IGDIObject.h"
class wxCursor;

namespace kxf
{
	class GDIIcon;
	class GDIBitmap;
}
namespace kxf::Drawing
{
	enum class StockCursor
	{
		None = 0,
		
		ArrowLeft = 1,
		ArrowRight = 2,
		ArrowWait = 27,
		ArrowQuestion = 16,

		PointLeft = 14,
		PointRight = 15,

		LeftButton = 8,
		MiddleButton = 10,
		MiddleRight = 17,

		SizeAny = 22,
		SizeRightTopLeftBottom = 18,
		SizeLeftTopRightBottom = 20,
		SizeTopBottom = 19,
		SizeLeftRight = 21,

		Char = 4,
		Cross = 5,
		Hand = 6,
		NoEntry = 11,
		Magnifier = 9,
		Wait = 24,
		Watch = 25,
		IBeam = 7,
		Bullseye = 3,
		PaintPrush = 12,
		SprayCan = 23,
		Pencil = 13,
		Blank = 26,

		Default = ArrowLeft
	};
}

namespace kxf
{
	class KXF_API GDICursor: public RTTI::DynamicImplementation<GDICursor, IGDIObject, IImage2D>
	{
		kxf_RTTI_DeclareIID(GDICursor, {0xec12b28a, 0x111e, 0x4f00, {0x8c, 0xe0, 0xdd, 0xb, 0x18, 0x9, 0xf7, 0x5e}});

		private:
			UninitializedStorage<wxCursor, 32, 0> m_Cursor;
			Point m_HotSpot = Point::UnspecifiedPosition();

		private:
			std::optional<String> GetOption(const String& name) const override
			{
				return {};
			}
			void SetOption(const String& name, const String& value) override
			{
			}

		public:
			GDICursor();
			GDICursor(const wxCursor& other);

			GDICursor(const GDIIcon& other);
			GDICursor(const GDIBitmap& other);
			GDICursor(const GDICursor& other);
			GDICursor(const BitmapImage& other);

			~GDICursor();

		public:
			// IGDIObject
			bool IsNull() const override;
			bool IsSameAs(const IGDIObject& other) const override;
			std::shared_ptr<IGDIObject> CloneGDIObject() const override
			{
				return std::make_shared<GDICursor>(*m_Cursor);
			}

			void* GetHandle() const override;
			void* DetachHandle() override;
			void AttachHandle(void* handle) override;

			// IImage2D
			bool IsSameAs(const IImage2D& other) const override;
			std::shared_ptr<IImage2D> CloneImage2D() const override
			{
				return std::make_shared<GDICursor>(*m_Cursor);
			}

			Size GetSize() const override;
			ColorDepth GetColorDepth() const override;
			UniversallyUniqueID GetFormat() const override;

			bool Create(const Size& size) override;
			bool Load(IInputStream& stream, const UniversallyUniqueID& format = ImageFormat::Any, size_t index = npos);
			bool Save(IOutputStream& stream, const UniversallyUniqueID& format) const;

			std::optional<int> GetOptionInt(const String& name) const override;
			void SetOption(const String& name, int value) override;

			BitmapImage ToBitmapImage(const Size& size = Size::UnspecifiedSize(), InterpolationQuality interpolationQuality = InterpolationQuality::Default) const override;

			// GDICursor
			wxCursor& AsWXCursor() noexcept
			{
				return *m_Cursor;
			}
			const wxCursor& AsWXCursor() const noexcept
			{
				return *m_Cursor;
			}

			GDIIcon ToGDIIcon() const;
			GDIBitmap ToGDIBitmap(const Size& size = Size::UnspecifiedSize(), InterpolationQuality interpolationQuality = InterpolationQuality::Default) const;

			Point GetHotSpot() const;
			void SetHotSpot(Point hotSpot);

		public:
			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}

			GDICursor& operator=(const GDICursor& other);
	};
}

namespace kxf::Drawing
{
	KXF_API GDICursor GetStockCursor(Drawing::StockCursor cursor);
}
