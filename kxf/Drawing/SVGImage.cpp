#include "kxf-pch.h"
#include "SVGImage.h"
#include "BitmapImage.h"
#include "kxf/IO/IStream.h"
#include "kxf/IO/StreamReaderWriter.h"
#include <lunasvg.h>

namespace
{
	constexpr double g_DefaultDPI = 96.0;

	constexpr bool IsValidDPI(int dpi) noexcept
	{
		return dpi > 0;
	}
}

namespace kxf
{
	// SVGImage
	void SVGImage::CopyFrom(const SVGImage& other)
	{
		if (this != &other)
		{
			m_Document = other.m_Document;
			m_DPI = other.m_DPI;
		}
	}
	void SVGImage::MoveFrom(SVGImage& other) noexcept
	{
		if (this != &other)
		{
			m_Document = std::move(other.m_Document);
			m_DPI = std::exchange(other.m_DPI, -1);
		}
	}

	std::string SVGImage::Serialize() const
	{
		// TODO: Find a way to serialize this as XML
		return {};
	}
	bool SVGImage::Deserialize(const std::string& svgData)
	{
		m_Document = lunasvg::Document::loadFromData(svgData);
		return m_Document != nullptr;
	}

	SVGImage::SVGImage() = default;
	SVGImage::SVGImage(const SVGImage& other)
	{
		CopyFrom(other);
	}
	SVGImage::SVGImage(SVGImage&& other) noexcept
	{
		MoveFrom(other);
	}
	SVGImage::~SVGImage() = default;

	// IImage2D
	bool SVGImage::IsNull() const
	{
		return m_Document == nullptr || m_Document->rootElement().isNull();
	}
	bool SVGImage::IsSameAs(const IImage2D& other) const
	{
		if (this == &other)
		{
			return true;
		}
		else if (auto ptr = other.QueryInterface<SVGImage>())
		{
			return m_Document == ptr->m_Document;
		}
		return false;
	}
	std::shared_ptr<IImage2D> SVGImage::CloneImage2D() const
	{
		return nullptr;
	}

	bool SVGImage::Create(const Size& size)
	{
		// Nothing to do
		return true;
	}
	bool SVGImage::Load(IInputStream& stream, const UniversallyUniqueID& format, size_t index)
	{
		m_Document = nullptr;

		if ((format == ImageFormat::SVG || format == ImageFormat::Any) && (index == 0 || index == npos))
		{
			IO::InputStreamReader reader(stream);
			return Deserialize(reader.ReadStringUTF8(stream.GetSize().ToBytes()).ToUTF8());
		}
		return false;
	}
	bool SVGImage::Save(IOutputStream& stream, const UniversallyUniqueID& format) const
	{
		if (!IsNull() && format != ImageFormat::None)
		{
			if (format == ImageFormat::SVG || format == ImageFormat::Any)
			{
				auto svgData = Serialize();
				return stream.WriteAll(svgData.data(), svgData.size());
			}
			else if (BitmapImage bitmap = SVGImage::ToBitmapImage(SVGImage::GetSize()))
			{
				return bitmap.Save(stream, format);
			}
		}
		return false;
	}

	Size SVGImage::GetSize() const
	{
		if (!IsNull())
		{
			return SizeD(m_Document->width(), m_Document->height()).ConvertCeil<Size>();
		}
		else
		{
			return Size::UnspecifiedSize();
		}
	}
	ColorDepth SVGImage::GetColorDepth() const
	{
		return ColorDepthDB::BPP32;
	}
	UniversallyUniqueID SVGImage::GetFormat() const
	{
		return ImageFormat::SVG;
	}

	std::optional<int> SVGImage::GetOptionInt(const String& name) const
	{
		if (!IsNull())
		{
			if (name == ImageOption::Resolution || name == ImageOption::DPI)
			{
				if (IsValidDPI(m_DPI))
				{
					return m_DPI;
				}
			}
		}
		return {};
	}
	void SVGImage::SetOption(const String& name, int value)
	{
		if (name == ImageOption::Resolution || name == ImageOption::DPI)
		{
			m_DPI = IsValidDPI(value) ? value : -1;
		}
	}

	BitmapImage SVGImage::ToBitmapImage(const Size& size, InterpolationQuality interpolationQuality) const
	{
		if (!IsNull())
		{
			constexpr auto backgroundColor = Drawing::GetStockColor(StockColor::Transparent);

			lunasvg::Bitmap svgBitmap;
			if (size.IsFullySpecified())
			{
				if (size.IsAnyComponentNegative())
				{
					return {};
				}
				svgBitmap = m_Document->renderToBitmap(size.GetWidth(), size.GetHeight(), backgroundColor.GetRGBA());
			}
			else
			{
				svgBitmap = m_Document->renderToBitmap(0, 0, backgroundColor.GetRGBA());
			}

			if (const auto sourceData = svgBitmap.data())
			{
				// If we have no size specified set it to the SVG's default size
				Geometry::BasicSize<size_t> actualSize = {svgBitmap.width(), svgBitmap.height()};
				if (size.IsFullySpecified())
				{
					actualSize.SetWidth(size.GetWidth());
					actualSize.SetHeight(size.GetHeight());
				}

				static_assert(sizeof(PackedRGBA<uint8_t>) == 4 && alignof(PackedRGBA<uint8_t>) == alignof(uint8_t));

				BitmapImage image(actualSize);
				image.SetPixelDataRGBA(reinterpret_cast<const PackedRGBA<uint8_t>*>(sourceData));
				return image;
			}
		}
		return {};
	}

	// IVectorImage
	Rect SVGImage::GetBoundingBox() const
	{
		if (!IsNull())
		{
			auto box = m_Document->rootElement().getBBox();
			return RectD(box.x, box.y, box.w, box.h).ConvertCeil<Rect>();
		}
		return {};
	}
}

namespace kxf
{
	uint64_t BinarySerializer<SVGImage>::Serialize(IOutputStream& stream, const SVGImage& value) const
	{
		return Serialization::WriteObject(stream, value.Serialize());
	}
	uint64_t BinarySerializer<SVGImage>::Deserialize(IInputStream& stream, SVGImage& value) const
	{
		std::string buffer;
		auto read = Serialization::ReadObject(stream, buffer);
		value.Deserialize(buffer);

		return read;
	}
}
