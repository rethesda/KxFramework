#pragma once
#include "Common.h"
#include "GDIBrush.h"
#include "GDIPen.h"
#include "../GraphicsRenderer/IGraphicsRenderer.h"

namespace kxf
{
	class KX_API GDIGraphicsRenderer: public RTTI::ExtendInterface<GDIGraphicsRenderer, IGraphicsRenderer>
	{
		KxRTTI_DeclareIID(GDIGraphicsRenderer, {0x366d774d, 0x7570, 0x489d, {0xb5, 0xbc, 0x6, 0x8, 0xa6, 0xbc, 0x88, 0xa4}});

		friend class GDIGraphicsContext;

		protected:
			GDIPen ToGDIPen(const IGraphicsPen& pen) const;
			GDIBrush ToGDIBrush(const IGraphicsBrush& brush) const;

		public:
			GDIGraphicsRenderer() noexcept = default;

		public:
			// IGraphicsRenderer
			String GetName() const override;
			Version GetVersion() const override;

			std::unique_ptr<IGraphicsContext> CreateContext(std::shared_ptr<IGraphicsTexture> texture) override;
			std::unique_ptr<IGraphicsContext> CreateGDIContext(wxDC& dc) override;
			std::unique_ptr<IGraphicsContext> CreateWindowContext(wxWindow& window) override;
			std::unique_ptr<IGraphicsContext> CreateWindowClientContext(wxWindow& window) override;
			std::unique_ptr<IGraphicsContext> CreateWindowPaintContext(wxWindow& window) override;
			std::unique_ptr<IGraphicsContext> CreateMeasuringContext() override;

		public:
			// Transformation matrix
			std::shared_ptr<IGraphicsMatrix> CreateMatrix(float m11, float m12, float m21, float m22, float tx, float ty) override;

			// Pen and brush functions
			std::shared_ptr<IGraphicsPen> CreatePen(const Color& color, float width = 1.0f) override;
			std::shared_ptr<IGraphicsSolidBrush> CreateSolidBrush(const Color& color) override;
			std::shared_ptr<IGraphicsTextureBrush> CreateTextureBrush(const Image& image) override;
			std::shared_ptr<IGraphicsTextureBrush> CreateTextureBrush(const GDIBitmap& bitmap);
			std::shared_ptr<IGraphicsLinearGradientBrush> CreateLinearGradientBrush(const RectF& rect, const GradientStops& colors, std::shared_ptr<IGraphicsMatrix> transform = {}) override;
			std::shared_ptr<IGraphicsRadialGradientBrush> CreateRadialGradientBrush(const RectF& rect, const GradientStops& colors, std::shared_ptr<IGraphicsMatrix> transform = {}) override;

			// Path functions (not implemented)
			std::shared_ptr<IGraphicsPath> CreatePath() override
			{
				return nullptr;
			}

			// Texture functions
			std::shared_ptr<IGraphicsTexture> CreateTexture() override;
			std::shared_ptr<IGraphicsTexture> CreateTexture(const Image& image) override;
			std::shared_ptr<IGraphicsTexture> CreateTexture(const GDIBitmap& bitmap);
			std::shared_ptr<IGraphicsTexture> CreateTexture(const SizeF& size, const Color& color) override;

			// Text functions
			std::shared_ptr<IGraphicsFont> CreateFont() override;
			std::shared_ptr<IGraphicsFont> CreateFont(const GDIFont& font, const Color& color = Drawing::GetStockColor(StockColor::Black)) override;
			std::shared_ptr<IGraphicsFont> CreateFont(const SizeF& pixelSize, const String& faceName, const Color& color = Drawing::GetStockColor(StockColor::Black)) override;
	};
}
