#include "stdafx.h"
#include "BitmapTextRenderer.h"
#include "TextRenderer.h"
#include "BitmapRenderer.h"

namespace KxFramework::UI::DataView
{
	bool BitmapTextValue::FromAny(const wxAny& value)
	{
		return TextValue::FromAny(value) || BitmapValue::FromAny(value) || value.GetAs(this);
	}
}

namespace KxFramework::UI::DataView
{
	bool BitmapTextRenderer::SetValue(const wxAny& value)
	{
		m_Value.Clear();
		return m_Value.FromAny(value);
	}

	void BitmapTextRenderer::DrawCellContent(const wxRect& cellRect, CellState cellState)
	{
		const bool centerTextV = m_Value.IsOptionEnabled(BitmapTextValueOption::VCenterText);
		const int reservedWidth = m_Value.GetDefaultBitmapWidth();

		GetRenderEngine().DrawBitmapWithText(cellRect, cellState, 0, m_Value.GetText(), m_Value.GetBitmap(), centerTextV, reservedWidth);
	}
	wxSize BitmapTextRenderer::GetCellSize() const
	{
		RenderEngine renderEngine = GetRenderEngine();

		wxSize size;
		if (m_Value.HasText())
		{
			size += renderEngine.GetTextExtent(m_Value.GetText());
		}
		if (m_Value.HasBitmap())
		{
			const wxBitmap& bitmap = m_Value.GetBitmap();
			size.x += bitmap.GetWidth() + GetRenderEngine().FromDIPX(renderEngine.GetInterTextSpacing());
			if (size.y < bitmap.GetHeight())
			{
				size.y = bitmap.GetHeight();
			}
		}
		else if (m_Value.IsDefaultBitmapWidthSpecified())
		{
			size.x += m_Value.GetDefaultBitmapWidth() + renderEngine.FromDIPX(renderEngine.GetInterTextSpacing());
		}
		return size;
	}
}