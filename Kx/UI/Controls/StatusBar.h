#pragma once
#include "Kx/UI/Common.h"
#include "Kx/UI/WindowRefreshScheduler.h"
#include <wx/statusbr.h>
#include <wx/systhemectrl.h>

namespace kxf::UI
{
	enum class StatusBarStyle
	{
		None = 0,

		SizeGrip = wxSTB_SIZEGRIP,
		ShowTips = wxSTB_SHOW_TIPS,

		EllipsizeStart = wxSTB_ELLIPSIZE_START,
		EllipsizeMiddle = wxSTB_ELLIPSIZE_MIDDLE,
		EllipsizeEnd = wxSTB_ELLIPSIZE_END,
	};
}
namespace kxf
{
	Kx_DeclareFlagSet(UI::StatusBarStyle);
}

namespace kxf::UI
{
	class KX_API StatusBar: public wxSystemThemedControl<WindowRefreshScheduler<wxStatusBar>>
	{
		public:
			static constexpr FlagSet<StatusBarStyle> DefaultStyle = CombineFlags<StatusBarStyle>(StatusBarStyle::SizeGrip|StatusBarStyle::ShowTips|StatusBarStyle::EllipsizeEnd, WindowStyle::FullRepaintOnResize);;

		private:
			void SetFieldsCount(int count, const int* widths) override;

		public:
			StatusBar() = default;
			StatusBar(wxWindow* parent,
					  wxWindowID id,
					  int fieldCount = 1,
					  FlagSet<StatusBarStyle> style = DefaultStyle
			)
			{
				Create(parent, id, fieldCount, style);
			}
			bool Create(wxWindow* parent,
						wxWindowID id,
						int fieldCount = 1,
						FlagSet<StatusBarStyle> style = DefaultStyle
			);

		public:
			void SetStatusWidth(int width, int index);
			void SetStatusWidths(const std::vector<int>& widths)
			{
				wxStatusBar::SetStatusWidths((int)widths.size(), widths.data());
			}
		
			virtual void SetFieldsCount(int count)
			{
				wxStatusBar::SetFieldsCount(count);
			}
			virtual void SetFieldsCount(const std::vector<int>& widths)
			{
				wxStatusBar::SetFieldsCount(std::min<size_t>(widths.size(), GetFieldsCount()), widths.data());
			}

		public:
			wxDECLARE_DYNAMIC_CLASS(StatusBar);
	};
}
