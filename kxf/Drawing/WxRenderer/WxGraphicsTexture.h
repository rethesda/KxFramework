#pragma once
#include "Common.h"
#include "WxGraphicsRenderer.h"
#include "../Image.h"
#include "../GraphicsRenderer/IGraphicsTexture.h"
#include <wx/graphics.h>

namespace kxf
{
	class KX_API WxGraphicsTexture: public RTTI::ExtendInterface<WxGraphicsTexture, IGraphicsTexture>
	{
		KxRTTI_DeclareIID(WxGraphicsTexture, {0x4c8a8b87, 0x9696, 0x4682, {0xb1, 0x8f, 0xc9, 0x55, 0x1f, 0xaa, 0xc, 0xd2}});

		protected:
			WxGraphicsRenderer* m_Renderer = nullptr;
			wxGraphicsBitmap m_Graphics;
			Image m_Image;

			bool m_Initialized = false;

		private:
			void Initialize()
			{
				if (!m_Initialized)
				{
					if (m_Image)
					{
						m_Image.InitAlpha();
						m_Graphics = m_Renderer->Get().CreateBitmapFromImage(m_Image.ToWxImage());
					}
					else
					{
						m_Graphics = {};
					}
					m_Initialized = true;
				}
			}
			void Invalidate()
			{
				m_Initialized = false;
			}

		public:
			WxGraphicsTexture() noexcept = default;
			WxGraphicsTexture(WxGraphicsRenderer& rendrer)
				:m_Renderer(&rendrer)
			{
			}
			WxGraphicsTexture(WxGraphicsRenderer& rendrer, const Image& image)
				:m_Renderer(&rendrer), m_Image(image.ToBitmap())
			{
			}
			WxGraphicsTexture(WxGraphicsRenderer& rendrer, const SizeF& size, const Color& color)
				:m_Renderer(&rendrer), m_Image(size)
			{
				if (color)
				{
					m_Image.InitAlpha();
					m_Image.SetRGBA(m_Image.GetSize(), color);
				}
			}

		public:
			// IGraphicsObject
			bool IsNull() const override
			{
				return !m_Renderer || m_Image.IsNull();
			}
			bool IsSameAs(const IGraphicsObject& other) const override
			{
				if (this == &other)
				{
					return true;
				}
				else if (auto object = other.QueryInterface<WxGraphicsTexture>())
				{
					return m_Image.IsSameAs(object->m_Image);
				}
				return false;
			}
			std::unique_ptr<IGraphicsObject> CloneGraphicsObject() const override
			{
				return std::make_unique<WxGraphicsTexture>(*this);
			}

			WxGraphicsRenderer& GetRenderer() override
			{
				return *m_Renderer;
			}
			void* GetNativeHandle() const
			{
				return m_Graphics.GetGraphicsData();
			}

			// IGraphicsTexture
			SizeF GetPPI() const override
			{
				return SizeF::UnspecifiedSize();
			}
			SizeF GetSize() const override
			{
				return m_Image.GetSize();
			}
			float GetWidth() const override
			{
				return m_Image.GetWidth();
			}
			float GetHeight() const override
			{
				return m_Image.GetHeight();
			}
			ColorDepth GetColorDepth() const override
			{
				return m_Image.GetColorDepth();
			}

			bool Load(IInputStream& stream, ImageFormat format = ImageFormat::Any) override
			{
				if (m_Image.Load(stream, format))
				{
					Invalidate();
					return true;
				}
				return false;
			}
			bool Save(IOutputStream& stream, ImageFormat format) const override
			{
				return m_Image.Save(stream, format);
			}

			std::shared_ptr<IGraphicsTexture> GetSubTexture(const RectF& rect) const override
			{
				return std::make_shared<WxGraphicsTexture>(*m_Renderer, m_Image.GetSubImage(rect));
			}
			void Rescale(const SizeF& size, InterpolationQuality interpolationQuality) override
			{
				m_Image.RescaleThis(size, interpolationQuality).ToBitmap();
				Invalidate();
			}

			Image ToImage() const override
			{
				return ToImage();
			}
			bool FromImage(const Image& image)
			{
				m_Image = image;
				Invalidate();

				return !m_Image.IsNull();
			}

			// WxGraphicsTexture
			const wxGraphicsBitmap& Get() const
			{
				const_cast<WxGraphicsTexture&>(*this).Initialize();
				return m_Graphics;
			}
			wxGraphicsBitmap& Get()
			{
				Initialize();
				return m_Graphics;
			}

			const Image& GetImage() const
			{
				return m_Image;
			}
			Image& GetImage()
			{
				return m_Image;
			}

		public:
			explicit operator bool() const
			{
				return !IsNull();
			}
			bool operator!() const
			{
				return IsNull();
			}
	};
}
