#include "stdafx.h"
#include "WxGraphicsContext.h"
#include "WxGraphicsTexture.h"
#include "WxGraphicsBrush.h"
#include "WxGraphicsPen.h"
#include "WxGraphicsFont.h"
#include "WxGraphicsPath.h"
#include <wx/msw/dc.h>

namespace kxf
{
	WxGraphicsContext::ChangeTextParameters::ChangeTextParameters(WxGraphicsContext& gc, const IGraphicsFont& font, const IGraphicsBrush& brush):m_Context(*gc.m_Context)
	{
		if (font || brush)
		{
			m_OldFont = gc.MakeGCFont();
			m_Context.SetFont(gc.MakeGCFont(font, brush));
		}
		else
		{
			gc.UpdateCurrentFont();
		}
	}
	WxGraphicsContext::ChangeTextParameters::~ChangeTextParameters()
	{
		if (!m_OldFont.IsNull())
		{
			m_Context.SetFont(m_OldFont);
		}
	}

	WxGraphicsContext::ChangeDrawParameters::ChangeDrawParameters(WxGraphicsContext& gc, const IGraphicsBrush& brush, const IGraphicsPen& pen):m_Context(*gc.m_Context)
	{
		if (brush)
		{
			if (gc.m_CurrentBrush && *gc.m_CurrentBrush)
			{
				m_OldBrush = gc.m_CurrentBrush->QueryInterface<WxGraphicsBrush>()->Get();
			}
			m_Context.SetBrush(brush.QueryInterface<WxGraphicsBrush>()->Get());
		}
		if (pen)
		{
			if (gc.m_CurrentPen && *gc.m_CurrentPen)
			{
				m_OldPen = gc.m_CurrentPen->QueryInterface<WxGraphicsPen>()->Get();
			}
			m_Context.SetPen(pen.QueryInterface<WxGraphicsPen>()->Get());
		}
	}
	WxGraphicsContext::ChangeDrawParameters::~ChangeDrawParameters()
	{
		if (!m_OldBrush.IsNull())
		{
			m_Context.SetBrush(m_OldBrush);
		}
		if (!m_OldPen.IsNull())
		{
			m_Context.SetPen(m_OldPen);
		}
	}
}

namespace kxf
{
	void WxGraphicsContext::SetupDC()
	{
		if (m_GCDC)
		{
			m_GCDC.SetMapMode(GDIMappingMode::Text);
			m_GCDC.SetTextBackground(Drawing::GetStockColor(StockColor::Transparent));
		}
	}
	void WxGraphicsContext::CopyAttributesFromDC(const GDIContext& dc)
	{
		if (dc)
		{
			SetFont(std::make_shared<WxGraphicsFont>(*m_Renderer, dc.GetFont()));
			SetFontBrush(std::make_shared<WxGraphicsSolidBrush>(*m_Renderer, dc.GetTextForeground()));
			SetBrush(std::make_shared<WxGraphicsSolidBrush>(*m_Renderer, dc.GetBrush()));
			SetPen(std::make_shared<WxGraphicsPen>(*m_Renderer, dc.GetPen()));
		}
	}

	wxGraphicsFont WxGraphicsContext::MakeGCFont() const
	{
		return m_Renderer->Get().CreateFont(m_CurrentFont.ToWxFont(), m_CurrentFontColor);
	}
	wxGraphicsFont WxGraphicsContext::MakeGCFont(const IGraphicsFont& font, const Color& color) const
	{
		GDIFont gdiFont;
		if (auto fontWx = font.QueryInterface<WxGraphicsFont>())
		{
			gdiFont = fontWx->GetFont();
		}
		else
		{
			gdiFont = font.ToFont();
		}
		return m_Renderer->Get().CreateFont(gdiFont.ToWxFont(), color);
	}
	wxGraphicsFont WxGraphicsContext::MakeGCFont(const IGraphicsFont& font, const IGraphicsBrush& brush) const
	{
		Color color = m_CurrentFontColor;
		if (auto solidBrush = brush.QueryInterface<IGraphicsSolidBrush>())
		{
			color = solidBrush->GetColor();
		}

		return MakeGCFont(font, color);
	}

	void WxGraphicsContext::UpdateCurrentFont()
	{
		if (!m_CurrentFontValid)
		{
			m_Context->SetFont(MakeGCFont());
			m_CurrentFontValid = true;
		}
	}

	// Feature support
	FlagSet<GraphicsContextFeature> WxGraphicsContext::GetSupportedFeatures() const
	{
		if (m_Context)
		{
			// Assume we support all operations (except bounding box), we can't check that anyway
			FlagSet<GraphicsContextFeature> features;
			features.Add(GraphicsContextFeature::ClippingRegion);
			features.Add(GraphicsContextFeature::TransformationMatrix);
			features.Add(GraphicsContextFeature::DrawText);
			features.Add(GraphicsContextFeature::DrawShape);
			features.Add(GraphicsContextFeature::DrawTexture);
			features.Add(GraphicsContextFeature::TextMeasurement);
			features.Add(GraphicsContextFeature::Antialiasing);
			features.Add(GraphicsContextFeature::Interpolation);
			features.Add(GraphicsContextFeature::Layers);
			features.Add(GraphicsContextFeature::States);

			return features;
		}
		return {};
	}

	// Clipping region functions
	void WxGraphicsContext::ClipBoxRegion(const RectF& rect)
	{
		m_Context->Clip(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
	}
	void WxGraphicsContext::ResetClipRegion()
	{
		m_Context->ResetClip();
	}
	RectF WxGraphicsContext::GetClipBox() const
	{
		wxDouble x = 0;
		wxDouble y = 0;
		wxDouble width = 0;
		wxDouble height = 0;
		m_Context->GetClipBox(&x, &y, &width, &height);

		return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)};
	}

	// Transformation matrix
	AffineMatrixF WxGraphicsContext::GetTransform() const
	{
		wxGraphicsMatrix transform = m_Context->GetTransform();
		if (!transform.IsNull())
		{
			return m_Renderer->ToAffineMatrix(transform);
		}
		return {};
	}
	bool WxGraphicsContext::SetTransform(const AffineMatrixF& transform)
	{
		m_Context->SetTransform(m_Renderer->FromAffineMatrix(transform));
		return true;
	}

	bool WxGraphicsContext::TransformInvert()
	{
		AffineMatrixF transform = m_Renderer->ToAffineMatrix(m_Context->GetTransform());
		if (transform.Invert())
		{
			m_Context->SetTransform(m_Renderer->FromAffineMatrix(transform));
			return true;
		}
		return false;
	}
	void WxGraphicsContext::TransformRotate(Angle angle)
	{
		m_Context->Rotate(angle.ToRadians());
	}
	void WxGraphicsContext::TransformScale(float xScale, float yScale)
	{
		m_Context->Scale(xScale, yScale);
	}
	void WxGraphicsContext::TransformTranslate(const SizeF& dxy)
	{
		m_Context->Translate(dxy.GetWidth(), dxy.GetHeight());
	}
	void WxGraphicsContext::TransformConcat(const AffineMatrixF& transform)
	{
		m_Context->ConcatTransform(m_Renderer->FromAffineMatrix(transform));
	}

	// Pen and brush functions
	std::shared_ptr<IGraphicsPen> WxGraphicsContext::GetPen() const
	{
		return m_CurrentPen;
	}
	void WxGraphicsContext::SetPen(std::shared_ptr<IGraphicsPen> pen)
	{
		if (pen && *pen)
		{
			if (auto penWx = pen->QueryInterface<WxGraphicsPen>())
			{
				m_Context->SetPen(penWx->Get());
				m_CurrentPen = std::move(pen);

				return;
			}
		}

		m_CurrentPen = {};
		m_Context->SetPen(wxGraphicsPen());
	}

	std::shared_ptr<IGraphicsBrush> WxGraphicsContext::GetBrush() const
	{
		return m_CurrentBrush;
	}
	void WxGraphicsContext::SetBrush(std::shared_ptr<IGraphicsBrush> brush)
	{
		if (brush && *brush)
		{
			if (auto brushWx = brush->QueryInterface<WxGraphicsBrush>())
			{
				m_Context->SetBrush(brushWx->Get());
				m_CurrentBrush = std::move(brush);

				return;
			}
		}

		m_CurrentBrush = {};
		m_Context->SetBrush(wxGraphicsBrush());
	}

	// Path functions
	void WxGraphicsContext::StrokePath(const IGraphicsPath& path)
	{
		m_Context->StrokePath(path.QueryInterface<WxGraphicsPath>()->Get());
	}
	void WxGraphicsContext::FillPath(const IGraphicsPath& path, PolygonFill fill)
	{
		if (auto fillMode = Drawing::Private::MapPolygonFill(fill))
		{
			m_Context->FillPath(path.QueryInterface<WxGraphicsPath>()->Get(), *fillMode);
		}
	}
	void WxGraphicsContext::DrawPath(const IGraphicsPath& path, PolygonFill fill)
	{
		if (auto fillMode = Drawing::Private::MapPolygonFill(fill))
		{
			m_Context->DrawPath(path.QueryInterface<WxGraphicsPath>()->Get(), *fillMode);
		}
	}

	// Texture functions
	void WxGraphicsContext::DrawTexture(const IGraphicsTexture& texture, const RectF& rect)
	{
		object_ptr<const WxGraphicsTexture> textureWx;
		if (!rect.IsEmpty() && texture && texture.QueryInterface(textureWx))
		{
			if (m_Renderer->CanRescaleBitmapOnDraw() || textureWx->GetSize() == rect.GetSize())
			{
				m_Context->DrawBitmap(textureWx->Get(), rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
			}
			else
			{
				Image image = textureWx->GetImage().Rescale(rect.GetSize(), m_InterpolationQuality);
				m_Context->DrawBitmap(m_Renderer->Get().CreateBitmapFromImage(image.ToWxImage()), rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
			}
			CalcBoundingBox(rect);
		}
	}
	void WxGraphicsContext::DrawTexture(const Image& image, const RectF& rect)
	{
		if (image && !rect.IsEmpty())
		{
			if (m_Renderer->CanRescaleBitmapOnDraw() || SizeF(image.GetSize()) == rect.GetSize())
			{
				m_Context->DrawBitmap(image.ToBitmap().ToWxBitmap(), rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
			}
			else
			{
				Image image = image.Rescale(rect.GetSize(), m_InterpolationQuality);
				m_Context->DrawBitmap(m_Renderer->Get().CreateBitmapFromImage(image.ToWxImage()), rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
			}
			CalcBoundingBox(rect);
		}
	}

	// Text functions
	std::shared_ptr<IGraphicsFont> WxGraphicsContext::GetFont() const
	{
		if (m_CurrentFont)
		{
			return std::make_shared<WxGraphicsFont>(*m_Renderer, m_CurrentFont);
		}
		return nullptr;
	}
	void WxGraphicsContext::SetFont(std::shared_ptr<IGraphicsFont> font)
	{
		m_CurrentFont = {};
		if (font && *font)
		{
			if (auto fontWx = font->QueryInterface<WxGraphicsFont>())
			{
				m_CurrentFont = fontWx->GetFont();
			}
			else
			{
				m_CurrentFont = font->ToFont();
			}
		}
		InvalidateCurrentFont();
	}

	std::shared_ptr<IGraphicsBrush> WxGraphicsContext::GetFontBrush() const
	{
		if (m_CurrentFontColor)
		{
			return std::make_shared<WxGraphicsSolidBrush>(*m_Renderer, m_CurrentFontColor);
		}
		return nullptr;
	}
	void WxGraphicsContext::SetFontBrush(std::shared_ptr<IGraphicsBrush> brush)
	{
		m_CurrentFontColor = {};
		if (brush && *brush)
		{
			if (auto solidBrush = brush->QueryInterface<IGraphicsSolidBrush>())
			{
				m_CurrentFontColor = solidBrush->GetColor();
			}
		}
		InvalidateCurrentFont();
	}

	GraphicsTextExtent WxGraphicsContext::GetTextExtent(const String& text, const IGraphicsFont& font) const
	{
		wxGraphicsFont oldFont;
		if (font)
		{
			oldFont = MakeGCFont();
			m_Context->SetFont(MakeGCFont(font));
		}
		else
		{
			const_cast<WxGraphicsContext&>(*this).UpdateCurrentFont();
		}

		wxDouble width = 0;
		wxDouble height = 0;
		wxDouble descent = 0;
		wxDouble externalLeading = 0;
		m_Context->GetTextExtent(text, &width, &height, &descent, &externalLeading);

		if (!oldFont.IsNull())
		{
			m_Context->SetFont(oldFont);
		}
		return {SizeF(width, height), GraphicsFontMetrics(0, 0, descent, 0, 0, externalLeading)};
	}
	std::vector<float> WxGraphicsContext::GetPartialTextExtent(const String& text, const IGraphicsFont& font) const
	{
		wxGraphicsFont oldFont;
		if (font)
		{
			oldFont = MakeGCFont();
			m_Context->SetFont(MakeGCFont(font));
		}
		else
		{
			const_cast<WxGraphicsContext&>(*this).UpdateCurrentFont();
		}

		wxArrayDouble gcWidths;
		m_Context->GetPartialTextExtents(text, gcWidths);

		std::vector<float> widths;
		widths.resize(gcWidths.size());
		std::copy(gcWidths.begin(), gcWidths.end(), widths.begin());

		if (!oldFont.IsNull())
		{
			m_Context->SetFont(oldFont);
		}
		return widths;
	}

	void WxGraphicsContext::DrawText(const String& text, const PointF& point, const IGraphicsFont& font, const IGraphicsBrush& brush)
	{
		if (!text.IsEmpty())
		{
			ChangeTextParameters textParametrs(*this, font, brush);

			m_Context->DrawText(text, point.GetX(), point.GetY());
			CalcBoundingBox(point);
		}
	}
	void WxGraphicsContext::DrawRotatedText(const String& text, const PointF& point, Angle angle, const IGraphicsFont& font, const IGraphicsBrush& brush)
	{
		if (!text.IsEmpty())
		{
			ChangeTextParameters textParametrs(*this, font, brush);

			m_Context->DrawText(text, point.GetX(), point.GetY(), angle.ToRadians());
			CalcBoundingBox(point);
		}
	}
	RectF WxGraphicsContext::DrawLabel(const String& text, const RectF& rect, const IGraphicsTexture& icon, const IGraphicsFont& font, const IGraphicsBrush& brush, FlagSet<Alignment> alignment, size_t acceleratorIndex)
	{
		if (!rect.IsEmpty() && (!text.IsEmpty() || icon))
		{
			ChangeTextParameters textParametrs(*this, font, brush);

			Rect boundingBox;
			if (icon)
			{
				boundingBox = m_GCDC.DrawLabel(text, rect, icon.QueryInterface<WxGraphicsTexture>()->GetImage(), alignment, acceleratorIndex);
			}
			else
			{
				boundingBox = m_GCDC.DrawLabel(text, rect, alignment, acceleratorIndex);
			}

			CalcBoundingBox(boundingBox);
			return boundingBox;
		}
		return {};
	}

	// Drawing functions
	void WxGraphicsContext::Clear(const IGraphicsBrush& brush)
	{
		WxGraphicsContext::DrawRectangle({{0, 0}, GetSize()}, brush, m_Renderer->GetTransparentPen());
	}
	void WxGraphicsContext::DrawCircle(const PointF& pos, float radius, const IGraphicsBrush& brush, const IGraphicsPen& pen)
	{
		if (radius != 0)
		{
			ChangeDrawParameters drawParameters(*this, brush, pen);

			RectF rect(pos.GetX() - radius, pos.GetY() - radius, 2 * radius, 2 * radius);
			m_Context->DrawEllipse(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
			CalcBoundingBox(rect);
		}
	}
	void WxGraphicsContext::DrawEllipse(const RectF& rect, const IGraphicsBrush& brush, const IGraphicsPen& pen)
	{
		if (!rect.IsEmpty())
		{
			ChangeDrawParameters drawParameters(*this, brush, pen);

			m_Context->DrawEllipse(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
			CalcBoundingBox(rect);
		}
	}
	void WxGraphicsContext::DrawRectangle(const RectF& rect, const IGraphicsBrush& brush, const IGraphicsPen& pen)
	{
		if (!rect.IsEmpty())
		{
			ChangeDrawParameters drawParameters(*this, brush, pen);

			m_Context->DrawRectangle(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
			CalcBoundingBox(rect);
		}
	}
	void WxGraphicsContext::DrawRoundedRectangle(const RectF& rect, float radius, const IGraphicsBrush& brush, const IGraphicsPen& pen)
	{
		if (!rect.IsEmpty())
		{
			ChangeDrawParameters drawParameters(*this, brush, pen);

			m_Context->DrawRoundedRectangle(rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight(), radius);
			CalcBoundingBox(rect);
		}
	}
	void WxGraphicsContext::DrawLine(const PointF& point1, const PointF& point2, const IGraphicsPen& pen)
	{
		ChangeDrawParameters drawParameters(*this, NullGraphicsBrush, pen);

		m_Context->StrokeLine(point1.GetX(), point1.GetY(), point2.GetX(), point2.GetY());
		CalcBoundingBox(point1);
		CalcBoundingBox(point2);
	}
	void WxGraphicsContext::DrawPolyLine(const PointF* points, size_t count, const IGraphicsPen& pen)
	{
		if (count != 0)
		{
			ChangeDrawParameters drawParameters(*this, NullGraphicsBrush, pen);

			std::vector<wxPoint2DDouble> gcPoints;
			gcPoints.resize(count);
			std::copy_n(points, count, gcPoints.begin());
			m_Context->StrokeLines(gcPoints.size(), gcPoints.data());

			for (size_t i = 0; i < count; i++)
			{
				CalcBoundingBox(points[i]);
			}
		}
	}
	void WxGraphicsContext::DrawDisconnectedLines(const PointF* startPoints, const PointF* endPoints, size_t count, const IGraphicsPen& pen)
	{
		if (count != 0)
		{
			ChangeDrawParameters drawParameters(*this, NullGraphicsBrush, pen);

			for (size_t i = 0; i < count; i++)
			{
				m_Context->StrokeLine(startPoints[i].GetX(), startPoints[i].GetY(), endPoints[i].GetX(), endPoints[i].GetY());

				CalcBoundingBox(startPoints[i]);
				CalcBoundingBox(endPoints[i]);
			}
		}
	}

	// Getting and setting parameters
	SizeF WxGraphicsContext::GetSize() const
	{
		wxDouble width = 0;
		wxDouble height = 0;
		m_Context->GetSize(&width, &height);

		return {static_cast<float>(width), static_cast<float>(height)};
	}
	SizeF WxGraphicsContext::GetPPI() const
	{
		wxDouble width = 0;
		wxDouble height = 0;
		m_Context->GetDPI(&width, &height);

		return {static_cast<float>(width), static_cast<float>(height)};
	}
	wxWindow* WxGraphicsContext::GetWindow() const
	{
		return m_Context->GetWindow();
	}

	AntialiasMode WxGraphicsContext::GetAntialiasMode() const
	{
		switch (m_Context->GetAntialiasMode())
		{
			case wxANTIALIAS_NONE:
			{
				return AntialiasMode::None;
			}
			case wxANTIALIAS_DEFAULT:
			{
				return AntialiasMode::Default;
			}
		};
	}
	void WxGraphicsContext::SetAntialiasMode(AntialiasMode mode)
	{
		if (mode == AntialiasMode::None)
		{
			m_Context->SetAntialiasMode(wxANTIALIAS_NONE);
		}
		else
		{
			m_Context->SetAntialiasMode(wxANTIALIAS_DEFAULT);
		}
	}

	CompositionMode WxGraphicsContext::GetCompositionMode() const
	{
		return m_CompositionMode;
	}
	void WxGraphicsContext::SetCompositionMode(CompositionMode mode)
	{
		m_CompositionMode = mode;
		switch (mode)
		{
			case CompositionMode::None:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_INVALID);
				break;
			}
			case CompositionMode::Clear:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_CLEAR);
				break;
			}
			case CompositionMode::Add:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_ADD);
				break;
			}
			case CompositionMode::Xor:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_XOR);
				break;
			}
			case CompositionMode::Source:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_SOURCE);
				break;
			}
			case CompositionMode::Over:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_OVER);
				break;
			}
			case CompositionMode::In:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_IN);
				break;
			}
			case CompositionMode::Out:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_OUT);
				break;
			}
			case CompositionMode::Atop:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_ATOP);
				break;
			}
			case CompositionMode::Dest:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_DEST);
				break;
			}
			case CompositionMode::DestOver:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_DEST_OVER);
				break;
			}
			case CompositionMode::DestIn:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_DEST_IN);
				break;
			}
			case CompositionMode::DestOut:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_DEST_OUT);
				break;
			}
			case CompositionMode::DestAtop:
			{
				m_Context->SetCompositionMode(wxCOMPOSITION_DEST_ATOP);
				break;
			}
		};
	}

	InterpolationQuality WxGraphicsContext::GetInterpolationQuality() const
	{
		return m_InterpolationQuality;
	}
	void WxGraphicsContext::SetInterpolationQuality(InterpolationQuality quality)
	{
		m_InterpolationQuality = quality;
		switch (quality)
		{
			case InterpolationQuality::None:
			{
				m_Context->SetInterpolationQuality(wxINTERPOLATION_NONE);
				break;
			}
			case InterpolationQuality::Default:
			{
				m_Context->SetInterpolationQuality(wxINTERPOLATION_DEFAULT);
				break;
			}
			case InterpolationQuality::FastestAvailable:
			case InterpolationQuality::NearestNeighbor:
			{
				m_Context->SetInterpolationQuality(wxINTERPOLATION_FAST);
				break;
			}
			case InterpolationQuality::Bilinear:
			case InterpolationQuality::Bicubic:
			{
				m_Context->SetInterpolationQuality(wxINTERPOLATION_GOOD);
				break;
			}
			case InterpolationQuality::BestAvailable:
			{
				m_Context->SetInterpolationQuality(wxINTERPOLATION_BEST);
				break;
			}
		};
	}

	// Bounding box functions
	RectF WxGraphicsContext::GetBoundingBox() const
	{
		if (m_BoundingBox)
		{
			return *m_BoundingBox;
		}
		return {};
	}
	void WxGraphicsContext::CalcBoundingBox(const PointF& point)
	{
		if (m_BoundingBox)
		{
			m_BoundingBox->IncludePoint(point);
		}
		else
		{
			m_BoundingBox = RectF(point, point);
		}
	}
	void WxGraphicsContext::ResetBoundingBox()
	{
		m_BoundingBox = {};
	}
}

namespace kxf
{
	void WxGraphicsGDIContext::Initialize(WxGraphicsRenderer& rendrer, wxDC& dc)
	{
		m_DC = dc;
		m_Image = Image(m_DC.GetSize());
		m_Image.SetRGBA(m_Image.GetSize(), Drawing::GetStockColor(StockColor::Transparent));

		WxGraphicsContext::Initialize(rendrer, std::unique_ptr<wxGraphicsContext>(rendrer.Get().CreateContextFromImage(m_Image.ToWxImage())));
	}

	bool WxGraphicsGDIContext::FlushContent()
	{
		if (m_Context && m_DC)
		{
			m_Context->Flush();
			m_DC.DrawBitmap(m_Image.ToBitmap(), {0, 0});

			return true;
		}
		return false;
	}
	void WxGraphicsGDIContext::ResetContext()
	{
		m_DC = {};
	}
}