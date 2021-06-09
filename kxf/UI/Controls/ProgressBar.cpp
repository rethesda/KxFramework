#include "KxfPCH.h"
#include "ProgressBar.h"
#include "kxf/General/Format.h"

namespace kxf::UI
{
	wxIMPLEMENT_DYNAMIC_CLASS(ProgressBar, wxGauge);

	void ProgressBar::OnPaint(wxPaintEvent& event)
	{
		if (!m_InPulseMode && HasFlag(wxGA_TEXT))
		{
			wxPaintDC dc(this);
			wxGauge::OnPaint(event);
			Rect rect(Point(0, 0), GetSize());

			// Draw label
			const String label = GetLabel();
			if (!label.IsEmpty())
			{
				dc.DrawLabel(Format("{:.2f}", (static_cast<double>(DoGetValue()) / static_cast<double>(DoGetRange())) * 100.0), rect, ToInt(Alignment::Center));
			}
		}
	}

	bool ProgressBar::Create(wxWindow* parent,
							 wxWindowID id,
							 int range,
							 FlagSet<ProgressBarStyle> style
	)
	{
		if (wxGauge::Create(parent, id, range, Point::UnspecifiedPosition(), Size::UnspecifiedSize(), style.ToInt()))
		{
			Bind(wxEVT_PAINT, &ProgressBar::OnPaint, this);
			return true;
		}
		return false;
	}
}
