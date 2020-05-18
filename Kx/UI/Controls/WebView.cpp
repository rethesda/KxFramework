#include "stdafx.h"
#include "WebView.h"

namespace KxFramework::UI
{
	wxIMPLEMENT_ABSTRACT_CLASS(WebView, wxWebView);

	wxWebView* WebView::New(wxWindow* parent,
							  wxWindowID id,
							  const String& defaultURL,
							  const String& backend,
							  long style
	)
	{
		return wxWebView::New(parent, id, defaultURL, wxDefaultPosition, wxDefaultSize, backend, style);
	}
}