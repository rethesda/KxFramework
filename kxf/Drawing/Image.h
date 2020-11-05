#pragma once
#include "Common.h"
#include "ColorDepth.h"
#include "kxf/General/Common.h"
#include "kxf/General/Color.h"
#include <wx/image.h>

namespace kxf
{
	class IInputStream;
	class IOutputStream;
}

namespace kxf
{
	class KX_API Image: public RTTI::Interface<Image>
	{
		KxRTTI_DeclareIID(Image, {0x84c3ee9b, 0x62dc, 0x4d8c, {0x8c, 0x94, 0xd1, 0xbc, 0xaf, 0x68, 0xfb, 0xc1}});

		public:
			enum class BlendMode
			{
				BlendOver,
				BlendCompose,
			};
			enum class ResizeQuality
			{
				Default = wxIMAGE_QUALITY_NORMAL,
				BestAvailable = wxIMAGE_QUALITY_HIGH,

				Nearest = wxIMAGE_QUALITY_NEAREST,
				Bilinear = wxIMAGE_QUALITY_BILINEAR,
				Bicubic = wxIMAGE_QUALITY_BICUBIC,
				BoxAverage = wxIMAGE_QUALITY_BOX_AVERAGE,
			};

		public:
			static int GetImageCount(IInputStream& stream, ImageFormat format = ImageFormat::Any);

		private:
			wxImage m_Image;

		public:
			Image() = default;
			Image(const wxImage& other)
				:m_Image(other)
			{
			}
			
			Image(const Icon& other);
			Image(const Bitmap& other);
			Image(const Cursor& other);
			Image(const Image& other)
				:m_Image(other.m_Image)
			{
			}

			Image(const Size& size)
				:m_Image(size.GetWidth(), size.GetHeight(), false)
			{
			}
			Image(const Size& size, uint8_t* rgb)
				:m_Image(size.GetWidth(), size.GetHeight(), rgb, true)
			{
			}
			Image(const Size& size, uint8_t* rgb, uint8_t* alpha)
				:m_Image(size.GetWidth(), size.GetHeight(), rgb, alpha, true)
			{
			}
			Image(const Size& size, wxMemoryBuffer& rgb)
				:m_Image(size.GetWidth(), size.GetHeight(), static_cast<unsigned char*>(rgb.release()), false)
			{
			}
			Image(const Size& size, wxMemoryBuffer& rgb, wxMemoryBuffer& alpha)
				:m_Image(size.GetWidth(), size.GetHeight(), static_cast<unsigned char*>(rgb.release()), static_cast<unsigned char*>(alpha.release()), false)
			{
			}

			virtual ~Image() = default;

		public:
			// Image
			bool IsNull() const
			{
				return !m_Image.IsOk();
			}
			bool IsSameAs(const Image& other) const
			{
				return m_Image.IsSameAs(other.m_Image);
			}
			Image Clone() const
			{
				return m_Image.Copy();
			}

			bool Load(IInputStream& stream, ImageFormat format = ImageFormat::Any, int index = -1);
			bool Save(IOutputStream& stream, ImageFormat format);

			const wxImage& ToWxImage() const noexcept
			{
				return m_Image;
			}
			wxImage& ToWxImage() noexcept
			{
				return m_Image;
			}

			Cursor ToCursor(const Point& hotSpot = Point::UnspecifiedPosition()) const;
			Bitmap ToBitmap() const;
			Icon ToIcon() const;

			// Properties
			Size GetSize() const
			{
				return m_Image.IsOk() ? Size(m_Image.GetSize()) : Size::UnspecifiedSize();
			}
			ColorDepth GetDepth() const
			{
				return m_Image.HasAlpha() ? ColorDepthDB::BPP32 : ColorDepthDB::BPP24;
			}

			ImageFormat GetFormat() const
			{
				return static_cast<ImageFormat>(m_Image.GetType());
			}
			void SetFormat(ImageFormat format)
			{
				m_Image.SetType(static_cast<wxBitmapType>(format));
			}

			// Options
			std::optional<String> GetOption(const String& name) const
			{
				if (m_Image.HasOption(name))
				{
					return m_Image.GetOption(name);
				}
				return {};
			}
			std::optional<int> GetOptionInt(const String& name) const
			{
				if (m_Image.HasOption(name))
				{
					return m_Image.GetOptionInt(name);
				}
				return {};
			}

			void SetOption(const String& name, int value)
			{
				m_Image.SetOption(name, value);
			}
			void SetOption(const String& name, const String& value)
			{
				m_Image.SetOption(name, value);
			}

			// Color data
			const uint8_t* GetData() const
			{
				return m_Image.GetData();
			}
			uint8_t* GetData()
			{
				return m_Image.GetData();
			}
			void ClearData(uint8_t value = 0)
			{
				m_Image.Clear(value);
			}

			void SetData(wxMemoryBuffer& alpha)
			{
				m_Image.SetData(static_cast<unsigned char*>(alpha.release()), false);
			}
			void SetData(uint8_t* alpha)
			{
				m_Image.SetData(alpha, true);
			}
			void SetData(const Size& size, wxMemoryBuffer& alpha)
			{
				m_Image.SetData(static_cast<unsigned char*>(alpha.release()), size.GetWidth(), size.GetHeight(), false);
			}
			void SetData(const Size& size, uint8_t* alpha)
			{
				m_Image.SetData(alpha, size.GetWidth(), size.GetHeight(), true);
			}

			uint8_t GetRed(const Point& pos) const
			{
				return m_Image.GetRed(pos.GetX(), pos.GetY());
			}
			uint8_t GetGreen(const Point& pos) const
			{
				return m_Image.GetGreen(pos.GetX(), pos.GetY());
			}
			uint8_t GetBlue(const Point& pos) const
			{
				return m_Image.GetBlue(pos.GetX(), pos.GetY());
			}
			void SetRed(const Point& pos, uint8_t value)
			{
				m_Image.SetRGB(pos.GetX(), pos.GetY(), value, GetGreen(pos), GetBlue(pos));
			}
			void SetGreen(const Point& pos, uint8_t value)
			{
				m_Image.SetRGB(pos.GetX(), pos.GetY(), GetRed(pos), value, GetBlue(pos));
			}
			void SetBlue(const Point& pos, uint8_t value)
			{
				m_Image.SetRGB(pos.GetX(), pos.GetY(), GetRed(pos), GetGreen(pos), value);
			}

			PackedRGB<uint8_t> GetRGB(const Point& pos) const
			{
				return {GetRed(pos), GetGreen(pos), GetBlue(pos)};
			}
			void SetRGB(const Point& pos, const PackedRGB<uint8_t>& color)
			{
				m_Image.SetRGB(pos.GetX(), pos.GetY(), color.Red, color.Green, color.Blue);
			}
			void SetRGB(const Point& pos, const Color& color)
			{
				SetRGB(pos, color.GetFixed8().RemoveAlpha());
			}

			PackedRGBA<uint8_t> GetRGBA(const Point& pos) const
			{
				return {GetRed(pos), GetGreen(pos), GetBlue(pos), GetAlpha(pos)};
			}
			void SetRGBA(const Point& pos, const PackedRGBA<uint8_t>& color)
			{
				m_Image.SetRGB(pos.GetX(), pos.GetY(), color.Red, color.Green, color.Blue);
				m_Image.SetAlpha(pos.GetX(), pos.GetY(), color.Alpha);
			}
			void SetRGBA(const Point& pos, const Color& color)
			{
				SetRGBA(pos, color.GetFixed8());
			}

			void SetRGB(const Rect& rect, const PackedRGB<uint8_t>& color)
			{
				m_Image.SetRGB(rect, color.Red, color.Green, color.Blue);
			}
			void SetRGB(const Rect& rect, const Color& color)
			{
				SetRGB(rect, color.GetFixed8().RemoveAlpha());
			}

			void ReplaceRGB(const PackedRGB<uint8_t>& source, const PackedRGB<uint8_t>& target)
			{
				m_Image.Replace(source.Red, source.Green, source.Blue, target.Red, target.Green, target.Blue);
			}
			void ReplaceRGB(const Color& source, const Color& target)
			{
				ReplaceRGB(source.GetFixed8().RemoveAlpha(), target.GetFixed8().RemoveAlpha());
			}

			void RotateHue(Angle angle)
			{
				m_Image.RotateHue(angle.ToNormalized());
			}

			// Alpha
			bool HasAlpha() const
			{
				return m_Image.HasAlpha();
			}
			bool InitAlpha()
			{
				if (!m_Image.HasAlpha())
				{
					m_Image.InitAlpha();
					return true;
				}
				return false;
			}
			bool ClearAlpha()
			{
				if (m_Image.HasAlpha())
				{
					m_Image.ClearAlpha();
					return true;
				}
				return false;
			}

			const uint8_t* GetAlpha() const
			{
				return m_Image.GetAlpha();
			}
			uint8_t* GetAlpha()
			{
				return m_Image.GetAlpha();
			}
			uint8_t GetAlpha(const Point& pos) const
			{
				return m_Image.GetAlpha(pos.GetX(), pos.GetY());
			}

			void SetAlpha(wxMemoryBuffer& alpha)
			{
				m_Image.SetAlpha(static_cast<unsigned char*>(alpha.release()), false);
			}
			void SetAlpha(uint8_t* alpha)
			{
				m_Image.SetAlpha(alpha, true);
			}
			void SetAlpha(const Point& pos, uint8_t value)
			{
				m_Image.SetAlpha(pos.GetX(), pos.GetY(), value);
			}

			bool IsTransparent(const Point& pos, uint8_t threshold = 128) const
			{
				return m_Image.IsTransparent(pos.GetX(), pos.GetY(), threshold);
			}

			// Mask
			bool IsMaskEnabled() const
			{
				return m_Image.HasMask();
			}
			void EnableMask(bool enable = true)
			{
				m_Image.SetMask(enable);
			}
			
			PackedRGB<uint8_t> GetMask() const
			{
				return {m_Image.GetMaskRed(), m_Image.GetMaskGreen(), m_Image.GetMaskBlue()};
			}
			void SetMask(const PackedRGB<uint8_t>& color)
			{
				m_Image.SetMaskColour(color.Red, color.Green, color.Blue);
			}
			void SetMask(const Color& color)
			{
				SetMask(color.GetFixed8().RemoveAlpha());
			}
			bool SetMask(const Image& shape, const PackedRGB<uint8_t>& color)
			{
				return m_Image.SetMaskFromImage(shape.m_Image, color.Red, color.Green, color.Blue);
			}
			bool SetMask(const Image& shape, const Color& color)
			{
				return SetMask(shape.m_Image, color.GetFixed8().RemoveAlpha());
			}

			// Image manipulation functions
			Image GetSubImage(const Rect& rect) const
			{
				return m_Image.GetSubImage(rect);
			}
			Image Paste(const Image& image, const Point& pos, BlendMode blendMode = BlendMode::BlendOver)
			{
				wxImage copy = m_Image;
				copy.Paste(image.m_Image, pos.GetX(), pos.GetY());
				return copy;
			}

			Image Blur(int radius, Orientation orientation = Orientation::Both) const
			{
				switch (orientation)
				{
					case Orientation::Vertical:
					{
						return m_Image.BlurVertical(radius);
					}
					case Orientation::Horizontal:
					{
						return m_Image.BlurHorizontal(radius);
					}
					case Orientation::Both:
					{
						return m_Image.Blur(radius);
					}
				};
				return {};
			}
			Image Mirror(Orientation orientation) const
			{
				switch (orientation)
				{
					case Orientation::Vertical:
					{
						return m_Image.Mirror(false);
					}
					case Orientation::Horizontal:
					{
						return m_Image.Mirror(true);
					}
				};
				return {};
			}

			Image Rotate90(ClockDirection direction = ClockDirection::Clockwise) const
			{
				return m_Image.Rotate90(direction == ClockDirection::Clockwise);
			}
			Image Rotate180() const
			{
				return m_Image.Rotate180();
			}
			Image Rotate(Angle angle, const Point& rotationCenter, bool interpolate) const
			{
				Point offsetAfterRotation;
				return Rotate(angle, rotationCenter, interpolate, offsetAfterRotation);
			}
			Image Rotate(Angle angle, const Point& rotationCenter, bool interpolate, Point& offsetAfterRotation) const
			{
				wxPoint offset;
				Image result = m_Image.Rotate(angle.ToRadians(), rotationCenter, interpolate, &offset);
				offsetAfterRotation = offset;

				return result;
			}

			Image Resize(const Size& size, const Point& pos) const
			{
				return m_Image.Size(size, pos);
			}
			Image Resize(const Size& size, const Point& pos, const Color& backgroundColor) const
			{
				return Resize(size, pos, backgroundColor.GetFixed8().RemoveAlpha());
			}
			Image Resize(const Size& size, const Point& pos, const PackedRGB<uint8_t>& backgroundColor) const
			{
				return m_Image.Size(size, pos, backgroundColor.Red, backgroundColor.Green, backgroundColor.Blue);
			}

			Image Rescale(const Size& size, ResizeQuality quality) const
			{
				return m_Image.Scale(size.GetWidth(), size.GetHeight(), static_cast<wxImageResizeQuality>(quality));
			}

			// Conversion functions
			Image ConvertToDisabled(Angle brightness = Angle::FromNormalized(1)) const
			{
				return m_Image.ConvertToDisabled(static_cast<uint8_t>(brightness.ToNormalized() * 255));
			}
			Image ConvertToMonochrome(const PackedRGB<uint8_t>& makeWhite) const
			{
				return m_Image.ConvertToMono(makeWhite.Red, makeWhite.Green, makeWhite.Blue);
			}
			Image ConvertToMonochrome(const Color& makeWhite) const
			{
				return ConvertToMonochrome(makeWhite.GetFixed8().RemoveAlpha());
			}
			Image ConvertToGrayscale(const PackedRGB<float>& weight = ColorWeight::CCIR_601) const
			{
				return m_Image.ConvertToGreyscale(weight.Red, weight.Green, weight.Blue);
			}

			// Miscellaneous functions
			bool FindFirstUnusedColour(PackedRGB<uint8_t>& firstUnused, const PackedRGB<uint8_t>& startAt = {1, 0, 0}) const
			{
				return m_Image.FindFirstUnusedColour(&firstUnused.Red, &firstUnused.Green, &firstUnused.Blue, startAt.Red, startAt.Green, startAt.Blue);
			}
			Color FindFirstUnusedColour(const Color& startAt = PackedRGBA<uint8_t>{1, 0, 0, 255}) const
			{
				PackedRGB<uint8_t> firstUnused;
				if (FindFirstUnusedColour(firstUnused, startAt.GetFixed8().RemoveAlpha()))
				{
					return firstUnused.AddAlpha(255);
				}
				return {};
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

			Image& operator=(const Image& other)
			{
				m_Image = other.m_Image;

				return *this;
			}
	};
}