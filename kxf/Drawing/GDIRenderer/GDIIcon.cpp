#include "kxf-pch.h"
#include "GDIIcon.h"
#include "GDICursor.h"
#include "GDIBitmap.h"
#include "Private.h"
#include "kxf/Drawing/BitmapImage.h"
#include "kxf/wxWidgets/Setup-IncludeImage.h"

namespace kxf
{
	// GDIIcon
	GDIIcon::GDIIcon()
	{
		m_Icon.ConstructAligned();
	}
	GDIIcon::GDIIcon(const wxIcon& other)
	{
		m_Icon.ConstructAligned(other);
	}

	GDIIcon::GDIIcon(const GDIIcon& other)
	{
		m_Icon.ConstructAligned(*other.m_Icon);
	}
	GDIIcon::GDIIcon(const GDIBitmap& other)
	{
		m_Icon.ConstructAligned(*other.ToGDIIcon().m_Icon);
	}
	GDIIcon::GDIIcon(const GDICursor& other)
	{
		m_Icon.ConstructAligned(*other.ToGDIIcon().m_Icon);
	}
	GDIIcon::GDIIcon(const BitmapImage& other)
	{
		m_Icon.ConstructAligned(other.ToWXIcon());
	}
	GDIIcon::GDIIcon(const char* xbm, const Size& size)
	{
		m_Icon.ConstructAligned(xbm, size.GetWidth(), size.GetHeight());
	}

	GDIIcon::~GDIIcon()
	{
		m_Icon.Destroy();
	}

	// IGDIObject
	bool GDIIcon::IsNull() const
	{
		return !m_Icon.IsConstructed() || !m_Icon->IsOk();
	}
	bool GDIIcon::IsSameAs(const IGDIObject& other) const
	{
		if (this == &other)
		{
			return true;
		}
		else if (auto ptr = other.QueryInterface<GDIIcon>())
		{
			return m_Icon->IsSameAs(*ptr->m_Icon);
		}
		else
		{
			return m_Icon->GetHandle() == other.GetHandle();
		}
	}

	void* GDIIcon::GetHandle() const
	{
		return m_Icon->GetHandle();
	}
	void* GDIIcon::DetachHandle()
	{
		return Drawing::Private::DetachGDIImageHandle(*m_Icon);
	}
	void GDIIcon::AttachHandle(void* handle)
	{
		*m_Icon = wxIcon();

		if (handle)
		{
			m_Icon->CreateFromHICON(reinterpret_cast<WXHICON>(handle));
			Drawing::Private::AttachIconHandle(*m_Icon, handle, []()
			{
				return true;
			});
		}
	}

	// IImage2D
	bool GDIIcon::IsSameAs(const IImage2D& other) const
	{
		if (this == &other)
		{
			return true;
		}
		else if (auto ptr = other.QueryInterface<GDIIcon>())
		{
			return m_Icon->IsSameAs(*ptr->m_Icon);
		}
		return false;
	}

	Size GDIIcon::GetSize() const
	{
		return m_Icon->IsOk() ? Size(m_Icon->GetSize()) : Size::UnspecifiedSize();
	}
	ColorDepth GDIIcon::GetColorDepth() const
	{
		return m_Icon->GetDepth();
	}
	UniversallyUniqueID GDIIcon::GetFormat() const
	{
		return ImageFormat::ICO;
	}

	bool GDIIcon::Create(const Size& size)
	{
		BitmapImage image(size);
		m_Icon->CopyFromBitmap(image.ToWXBitmap());

		return m_Icon->IsOk();
	}
	bool GDIIcon::Load(IInputStream& stream, const UniversallyUniqueID& format, size_t index)
	{
		BitmapImage image;
		if (image.Load(stream, format, index == IImage2D::npos ? -1 : static_cast<int>(index)))
		{
			m_Icon->CopyFromBitmap(image.ToWXBitmap());
			return m_Icon->IsOk();
		}
		return false;
	}
	bool GDIIcon::Save(IOutputStream& stream, const UniversallyUniqueID& format) const
	{
		if (m_Icon->IsOk() && format != ImageFormat::Any && format != ImageFormat::None)
		{
			return ToBitmapImage().Save(stream, format);
		}
		return false;
	}

	BitmapImage GDIIcon::ToBitmapImage(const Size& size, InterpolationQuality interpolationQuality) const
	{
		return GDIIcon::ToGDIBitmap(size, interpolationQuality).ToBitmapImage(Size::UnspecifiedSize(), InterpolationQuality::None);
	}

	// GDIIcon
	GDICursor GDIIcon::ToGDICursor(const Point& hotSpot) const
	{
		if (m_Icon->IsOk())
		{
			GDICursor cursor(ToGDIBitmap());
			cursor.SetHotSpot(hotSpot);

			return cursor;
		}
		return {};
	}
	GDIBitmap GDIIcon::ToGDIBitmap(const Size& size, InterpolationQuality interpolationQuality) const
	{
		if (m_Icon->IsOk())
		{
			wxBitmap bitmap(*m_Icon, wxBitmapTransparency::wxBitmapTransparency_Always);
			if (!size.IsFullySpecified() || m_Icon->GetSize() == size)
			{
				return bitmap;
			}

			BitmapImage image = bitmap;
			image.Rescale(size, interpolationQuality);
			return image.ToWXBitmap();
		}
		return {};
	}

	GDIIcon GDIIcon::ConvertToDisabled(Angle brightness) const
	{
		return ToGDIBitmap().ConvertToDisabled(brightness);
	}

	GDIIcon& GDIIcon::operator=(const GDIIcon& other)
	{
		*m_Icon = *other.m_Icon;

		return *this;
	}
}
