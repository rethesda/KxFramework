#pragma once
#include "Host.h"

namespace KxSciter
{
	template<class TWindow>
	class WindowWrapper: public TWindow, public Host
	{
		protected:
			bool MSWHandleMessage(WXLRESULT* result, WXUINT msg, WXWPARAM wParam, WXLPARAM lParam) override
			{
				// Handle messages before native window
				if (Host::SciterPreHandleMessage(result, msg, wParam, lParam))
				{
					return true;
				}
				
				// Call native window procedure
				bool handled = TWindow::MSWHandleMessage(result, msg, wParam, lParam);

				// Post process messages
				if (Host::SciterPostHandleMessage(result, msg, wParam, lParam))
				{
					handled = true;
				}
				return handled;
			}
			void OnInternalIdle() override
			{
				// Base class version generates too much strain on message processing system
				// if there is more than one Sciter shown at the same time so let's just call
				// Host object idle handler. Base class version doesn't seems to be needed here.
				Host::OnInternalIdle();
			}

			wxSize DoGetBestClientSize() const override
			{
				return Host::GetBestSize();
			}

		public:
			WindowWrapper()
				:Host(static_cast<wxWindow&>(*this))
			{
			}
			WindowWrapper(const WindowWrapper&) = delete;

			template<class... Args>
			WindowWrapper(Args&&... arg)
				:TWindow(std::forward<Args>(arg)...), Host(static_cast<wxWindow&>(*this))
			{
				Host::Create();
			}

			template<class... Args>
			bool Create(Args&&... arg)
			{
				return TWindow::Create(std::forward<Args>(arg)...) && Host::Create();
			}
			
		public:
			bool Layout() override
			{
				const bool result = TWindow::Layout();
				Host::Update();
				return result;
			}
			void Update() override
			{
				TWindow::Update();
				Host::Update();
			}
			void Refresh(bool eraseBackground = true, const wxRect* rect = nullptr) override
			{
				TWindow::Refresh(eraseBackground, rect);
				Host::Update();
			}
			void UpdateWindowUI(long flags = wxUPDATE_UI_NONE) override
			{
				TWindow::UpdateWindowUI(flags);
				Host::Update();
			}

			bool IsSystemThemeEnabled() const
			{
				return Host::IsSystemThemeEnabled();
			}
			void EnableSystemTheme(bool enable = true)
			{
				Host::EnableSystemTheme(enable);
			}

			bool GetThemeEnabled() const override
			{
				return Host::IsSystemThemeEnabled();
			}
			void SetThemeEnabled(bool enable = true)
			{
				TWindow::SetThemeEnabled(true);
			}
			
			bool IsTransparentBackgroundSupported(wxString* reason = nullptr) const
			{
				return Host::IsTransparentBackgroundSupported(reason);;
			}
			bool HasTransparentBackground() override
			{
				return true;
			}
			bool CanSetTransparent() override
			{
				return true;
			}
			bool SetTransparent(wxByte alpha)
			{
				return Host::SetTransparency(alpha);
			}
			
			bool SetFont(const wxFont& font) override
			{
				Host::SetFont(font);
				return TWindow::SetFont(font);
			}
			bool SetBackgroundColour(const wxColour& color) override
			{
				TWindow::SetBackgroundColour(color);
				return Host::SetBackgroundColor(color);
			}
			bool SetForegroundColour(const wxColour& color) override
			{
				TWindow::SetForegroundColour(color);
				return Host::SetForegroundColor(color);
			}

			wxLayoutDirection GetLayoutDirection() const override
			{
				return Host::GetLayoutDirection();
			}
			void SetLayoutDirection(wxLayoutDirection value) override
			{
				TWindow::SetLayoutDirection(value);
				Host::SetLayoutDirection(value);
			}

			wxSize GetDPI() const override
			{
				return Host::GetDPI();
			}
			wxSize GetBestSize() const
			{
				return TWindow::GetBestSize();
			}

		public:
			WindowWrapper& operator=(const WindowWrapper&) = delete;
	};
}