#pragma once
#include "../Common.h"

namespace KxFramework::Localization::Private
{
	String LocalizeLabelString(StdID id, wxStockLabelQueryFlag flags = wxStockLabelQueryFlag::wxSTOCK_NOFLAGS);
	String LocalizeHelpString(StdID id, wxStockHelpStringClient client);
}
