#pragma once
#include "Common.h"
#include "IVectorImage.h"
#include "kxf/Serialization/BinarySerializer.h"

namespace lunasvg
{
	class Document;
}

namespace kxf
{
	class KXF_API SVGImage final: public RTTI::DynamicImplementation<SVGImage, IVectorImage>
	{
		kxf_RTTI_DeclareIID_Using(SVGImage, ImageFormat::SVG.ToNativeUUID());

		friend struct BinarySerializer<SVGImage>;

		private:
			std::shared_ptr<lunasvg::Document> m_Document;
			int m_DPI = -1;

		private:
			void CopyFrom(const SVGImage& other);
			void MoveFrom(SVGImage& other) noexcept;

			std::string Serialize() const;
			bool Deserialize(const std::string& svgData);

		public:
			SVGImage();
			SVGImage(const SVGImage& other);
			SVGImage(SVGImage&& other) noexcept;
			~SVGImage();

		public:
			// IImage2D
			bool IsNull() const;
			bool IsSameAs(const IImage2D& other) const override;
			std::shared_ptr<IImage2D> CloneImage2D() const override;

			bool Create(const Size& size);
			bool Load(IInputStream& stream, const UniversallyUniqueID& format = ImageFormat::Any, size_t index = npos);
			bool Save(IOutputStream& stream, const UniversallyUniqueID& format) const;

			Size GetSize() const override;
			ColorDepth GetColorDepth() const override;
			UniversallyUniqueID GetFormat() const override;

			std::optional<String> GetOption(const String& name) const override
			{
				return {};
			}
			std::optional<int> GetOptionInt(const String& name) const override;

			void SetOption(const String& name, int value) override;
			void SetOption(const String& name, const String& value) override
			{
			}

			BitmapImage ToBitmapImage(const Size& size = Size::UnspecifiedSize(), InterpolationQuality interpolationQuality = InterpolationQuality::Default) const override;

			// IVectorImage
			Rect GetBoundingBox() const override;

		public:
			explicit operator bool() const noexcept
			{
				return !IsNull();
			}
			bool operator!() const noexcept
			{
				return IsNull();
			}

			SVGImage& operator=(const SVGImage& other)
			{
				CopyFrom(other);
				return *this;
			}
			SVGImage& operator=(SVGImage&& other) noexcept
			{
				MoveFrom(other);
				return *this;
			}
	};
}

namespace kxf
{
	template<>
	struct KXF_API BinarySerializer<SVGImage> final
	{
		uint64_t Serialize(IOutputStream& stream, const SVGImage& value) const;
		uint64_t Deserialize(IInputStream& stream, SVGImage& value) const;
	};
}
