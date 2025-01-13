#include "kxf-pch.h"
#include "SVGImage.h"
#include "BitmapImage.h"
#include "kxf/IO/IStream.h"
#include "kxf/IO/StreamReaderWriter.h"
#include <lunasvg/svgdocument.h>

namespace
{
	constexpr double g_DefaultDPI = 96.0;

	constexpr bool IsValidDPI(int dpi) noexcept
	{
		return dpi > 0;
	}
	constexpr double ToSVGDPI(int dpi) noexcept
	{
		return IsValidDPI(dpi) ? dpi : g_DefaultDPI;
	}

	int FromSVGTime(double time) noexcept
	{
		return static_cast<int>(time * 1000.0);
	}
	double ToSVGTime(int time)
	{
		return time / 1000.0;
	}
}

namespace kxf
{
	class SVGImageImpl final: public lunasvg::SVGDocument {};
}

namespace kxf
{
	// SVGImage
	void SVGImage::CopyFrom(const SVGImage& other)
	{
		if (this != &other)
		{
			if (!other.IsNull())
			{
				m_Document->loadFromData(other.m_Document->toString());
				m_DPI = other.m_DPI;
			}
			else
			{
				*m_Document = {};
			}
		}
	}
	void SVGImage::MoveFrom(SVGImage& other) noexcept
	{
		static_assert(std::is_standard_layout_v<lunasvg::SVGDocument>, "standard layout expected");

		if (this != &other)
		{
			m_Document.Destroy();
			m_Document.Construct();
			m_Document.GetBuffer().MoveBuffer(other.m_Document.GetBuffer());

			m_DPI = std::exchange(other.m_DPI, -1);
		}
	}

	SVGImage::SVGImage()
	{
		m_Document.Construct();
	}
	SVGImage::SVGImage(const SVGImage& other)
	{
		m_Document.Construct();
		CopyFrom(other);
	}
	SVGImage::SVGImage(SVGImage&& other) noexcept
	{
		MoveFrom(other);

		if (!m_Document.IsConstructed())
		{
			m_Document.Construct();
		}
	}
	SVGImage::~SVGImage()
	{
		m_Document.Destroy();
	}

	// IImage2D
	bool SVGImage::IsNull() const
	{
		return !m_Document.IsConstructed();
	}
	bool SVGImage::IsSameAs(const IImage2D& other) const
	{
		if (this == &other)
		{
			return true;
		}
		else if (auto ptr = other.QueryInterface<SVGImage>())
		{
			return m_Document->impl() == ptr->m_Document->impl() || m_Document->toString() == ptr->m_Document->toString();
		}
		return false;
	}

	bool SVGImage::Create(const Size& size)
	{
		// Nothing to do
		return true;
	}
	bool SVGImage::Load(IInputStream& stream, const UniversallyUniqueID& format, size_t index)
	{
		*m_Document = {};

		if ((format == ImageFormat::SVG || format == ImageFormat::Any) && (index == 0 || index == npos))
		{
			IO::InputStreamReader reader(stream);
			if (m_Document->loadFromData(reader.ReadStringUTF8(stream.GetSize().ToBytes()).ToUTF8()))
			{
				return true;
			}
		}
		return false;
	}
	bool SVGImage::Save(IOutputStream& stream, const UniversallyUniqueID& format) const
	{
		if (m_Document && format != ImageFormat::None)
		{
			if (format == ImageFormat::SVG || format == ImageFormat::Any)
			{
				IO::OutputStreamWriter writer(stream);
				writer.WriteStringUTF8(String::FromUTF8(m_Document->toString()));
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
		return m_Document ? Size(m_Document->documentWidth(ToSVGDPI(m_DPI)), m_Document->documentHeight(ToSVGDPI(m_DPI))) : Size::UnspecifiedSize();
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
		if (m_Document)
		{
			if (name == ImageOption::Resolution || name == ImageOption::DPI)
			{
				if (IsValidDPI(m_DPI))
				{
					return m_DPI;
				}
			}
			else if (name == ImageOption::SVG::HasAnimation)
			{
				return m_Document->hasAnimation() ? 1 : 0;
			}
			else if (name == ImageOption::SVG::AnimationDuration)
			{
				return FromSVGTime(m_Document->animationDuration());
			}
			else if (name == ImageOption::SVG::CurrentTime)
			{
				return FromSVGTime(m_Document->currentTime());
			}
		}
		return {};
	}
	void SVGImage::SetOption(const String& name, int value)
	{
		if (name == ImageOption::Resolution || name == ImageOption::DPI)
		{
			m_DPI = value;
		}
		else if (name == ImageOption::SVG::CurrentTime)
		{
			m_Document->setCurrentTime(ToSVGTime(value), true);
		}
	}

	BitmapImage SVGImage::ToBitmapImage(const Size& size, InterpolationQuality interpolationQuality) const
	{
		if (m_Document)
		{
			lunasvg::Bitmap svgBitmap;
			if (size.IsFullySpecified())
			{
				if (size.IsAnyComponentNegative())
				{
					return {};
				}
				svgBitmap = m_Document->renderToBitmap(size.GetWidth(), size.GetHeight(), ToSVGDPI(m_DPI));
			}
			else
			{
				svgBitmap = m_Document->renderToBitmap(0, 0, ToSVGDPI(m_DPI));
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
		if (m_Document)
		{
			auto box = m_Document->getBBox(ToSVGDPI(m_DPI));
			return RectD(box.x, box.y, box.width, box.height).ConvertCeil<Rect>();
		}
		return {};
	}
}

namespace kxf
{
	uint64_t BinarySerializer<SVGImage>::Serialize(IOutputStream& stream, const SVGImage& value) const
	{
		return value.m_Document ? Serialization::WriteObject(stream, value.m_Document->toString()) : Serialization::WriteObject(stream, std::string());
	}
	uint64_t BinarySerializer<SVGImage>::Deserialize(IInputStream& stream, SVGImage& value) const
	{
		std::string buffer;
		auto read = Serialization::ReadObject(stream, buffer);

		*value.m_Document = {};
		value.m_Document->loadFromData(buffer);
		return read;
	}
}
