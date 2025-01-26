#include "kxf-pch.h"
#include "GDIBitmap.h"
#include "GDICursor.h"
#include "GDIIcon.h"
#include "Private.h"
#include "kxf/Drawing/BitmapImage.h"
#include "kxf/wxWidgets/Setup-IncludeImage.h"

namespace kxf
{
	// GDIBitmap
	void GDIBitmap::Initialize()
	{
		// This shouldn't be necessary, at least for 'ToBitmapImage' function
		// since it can detect whether the alpha is actually used or not.
		//m_Bitmap->UseAlpha(true);
	}

	GDIBitmap::GDIBitmap()
	{
		m_Bitmap.ConstructAligned();
		Initialize();
	}
	GDIBitmap::GDIBitmap(const wxBitmap& other)
	{
		m_Bitmap.ConstructAligned(other);
		Initialize();
	}
	GDIBitmap::GDIBitmap(const GDIIcon& other)
	{
		m_Bitmap.ConstructAligned(*other.ToGDIBitmap().m_Bitmap);
		Initialize();
	}
	GDIBitmap::GDIBitmap(const GDICursor& other)
	{
		m_Bitmap.ConstructAligned(*other.ToGDIBitmap().m_Bitmap);
		Initialize();
	}
	GDIBitmap::GDIBitmap(const GDIBitmap& other)
	{
		m_Bitmap.ConstructAligned(*other.m_Bitmap);
		Initialize();
	}
	GDIBitmap::GDIBitmap(const BitmapImage& other)
	{
		m_Bitmap.ConstructAligned(other.ToWXBitmap());
		Initialize();
	}
	GDIBitmap::GDIBitmap(const BitmapImage& other, const wxDC& dc)
	{
		m_Bitmap.ConstructAligned(other.AsWXImage(), dc);
		Initialize();
	}

	GDIBitmap::GDIBitmap(const Size& size, ColorDepth depth)
	{
		m_Bitmap.ConstructAligned(size.GetWidth(), size.GetHeight(), depth ? depth.GetValue() : -1);
		Initialize();
	}
	GDIBitmap::GDIBitmap(const Size& size, const wxDC& dc)
	{
		m_Bitmap.ConstructAligned(size.GetWidth(), size.GetHeight(), dc);
		Initialize();
	}
	GDIBitmap::GDIBitmap(const char* xbm, const Size& size, ColorDepth depth)
	{
		m_Bitmap.ConstructAligned(xbm, size.GetWidth(), size.GetHeight(), depth ? depth.GetValue() : -1);
		Initialize();
	}

	GDIBitmap::~GDIBitmap()
	{
		m_Bitmap.Destroy();
	}

	// IGDIObject
	bool GDIBitmap::IsNull() const
	{
		return !m_Bitmap.IsConstructed() || !m_Bitmap->IsOk();
	}
	bool GDIBitmap::IsSameAs(const IGDIObject& other) const
	{
		if (this == &other)
		{
			return true;
		}
		else if (auto ptr = other.QueryInterface<GDIBitmap>())
		{
			return m_Bitmap->IsSameAs(*ptr->m_Bitmap);
		}
		else
		{
			return m_Bitmap->GetHandle() == other.GetHandle();
		}
	}

	void* GDIBitmap::GetHandle() const
	{
		return m_Bitmap->GetHandle();
	}
	void* GDIBitmap::DetachHandle()
	{
		return Drawing::Private::DetachGDIImageHandle(*m_Bitmap);
	}
	void GDIBitmap::AttachHandle(void* handle)
	{
		*m_Bitmap = wxBitmap();

		if (handle)
		{
			BITMAP bmpData = {};
			if (::GetObjectW(handle, sizeof(bmpData), &bmpData) != 0)
			{
				m_Bitmap->InitFromHBITMAP(reinterpret_cast<WXHBITMAP>(handle), bmpData.bmWidth, bmpData.bmHeight, bmpData.bmBitsPixel);
				return;
			}
		}
		m_Bitmap->SetHandle(handle);
	}

	// IImage2D
	bool GDIBitmap::IsSameAs(const IImage2D& other) const
	{
		if (this == &other)
		{
			return true;
		}
		else if (auto ptr = other.QueryInterface<GDIBitmap>())
		{
			return m_Bitmap->IsSameAs(*ptr->m_Bitmap);
		}
		return false;
	}

	Size GDIBitmap::GetSize() const
	{
		return m_Bitmap->IsOk() ? Size(m_Bitmap->GetSize()) : Size::UnspecifiedSize();
	}
	ColorDepth GDIBitmap::GetColorDepth() const
	{
		return m_Bitmap->GetDepth();
	}
	UniversallyUniqueID GDIBitmap::GetFormat() const
	{
		return ImageFormat::BMP;
	}

	bool GDIBitmap::Create(const Size& size)
	{
		return m_Bitmap->Create(size, *ColorDepthDB::BPP32);
	}
	bool GDIBitmap::Load(IInputStream& stream, const UniversallyUniqueID& format, size_t index)
	{
		BitmapImage image;
		if (image.Load(stream, format))
		{
			*m_Bitmap = image.ToWXBitmap();
			return m_Bitmap->IsOk();
		}
		return false;
	}
	bool GDIBitmap::Save(IOutputStream& stream, const UniversallyUniqueID& format) const
	{
		if (m_Bitmap->IsOk() && format != ImageFormat::Any && format != ImageFormat::None)
		{
			return ToBitmapImage().Save(stream, format);
		}
		return false;
	}

	BitmapImage GDIBitmap::ToBitmapImage(const Size& size, InterpolationQuality interpolationQuality) const
	{
		if (m_Bitmap->IsOk())
		{
			if (!size.IsFullySpecified() || m_Bitmap->GetSize() == size)
			{
				return m_Bitmap->ConvertToImage();
			}
			else
			{
				BitmapImage image = m_Bitmap->ConvertToImage();
				image.Rescale(size, interpolationQuality);
				return image.ToWXBitmap();
			}
		}
		return {};
	}

	// GDIBitmap
	GDICursor GDIBitmap::ToGDICursor(const Point& hotSpot) const
	{
		if (m_Bitmap->IsOk())
		{
			GDICursor cursor(wxCursor(m_Bitmap->ConvertToImage()));
			cursor.SetHotSpot(hotSpot);

			return cursor;
		}
		return {};
	}
	GDIIcon GDIBitmap::ToGDIIcon() const
	{
		if (m_Bitmap->IsOk())
		{
			wxIcon icon;
			icon.CopyFromBitmap(*m_Bitmap);

			return icon;
		}
		return {};
	}

	GDIBitmap GDIBitmap::GetSubBitmap(const Rect& rect) const
	{
		if (m_Bitmap->IsOk())
		{
			return m_Bitmap->GetSubBitmap(rect);
		}
		return {};
	}
	GDIBitmap GDIBitmap::ConvertToDisabled(Angle brightness) const
	{
		if (m_Bitmap->IsOk())
		{
			return m_Bitmap->ConvertToDisabled(static_cast<uint8_t>(brightness.ToNormalized() * 255));
		}
		return {};
	}
	GDIBitmap GDIBitmap::GetScaled(const Size& size, InterpolationQuality interpolationQuality) const
	{
		if (m_Bitmap->IsOk())
		{
			if (!size.IsFullySpecified() || m_Bitmap->GetSize() == size)
			{
				return *m_Bitmap;
			}
			else
			{
				BitmapImage image = m_Bitmap->ConvertToImage();
				image.Rescale(size, interpolationQuality);
				return image.ToWXBitmap();
			}
		}
		return {};
	}

	void GDIBitmap::ForceAlpha()
	{
		m_Bitmap->UseAlpha(true);
	}
	void GDIBitmap::UpdateAlpha()
	{
		m_Bitmap->MSWUpdateAlpha();
	}

	kxf::GDIBitmap& GDIBitmap::operator=(const GDIBitmap& other)
	{
		*m_Bitmap = *other.m_Bitmap;

		return *this;
	}
}
