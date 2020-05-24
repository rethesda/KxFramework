#pragma once
#include "Kx/UI/Common.h"
#include <wx/webview.h>

namespace kxf::UI
{
	class KX_API WebView: public wxWebView
	{
		public:
			static constexpr XChar DefaultPage[] = wxS("about:blank");
			static constexpr int DefaultStyle = 0;

		public:
			static wxWebView* New(wxWindow* parent,
								  wxWindowID id,
								  const String& defaultURL = DefaultPage,
								  const String& backend = wxWebViewBackendDefault,
								  long style = DefaultStyle
			);

		public:
			wxDECLARE_ABSTRACT_CLASS(WebView);
	};
}
