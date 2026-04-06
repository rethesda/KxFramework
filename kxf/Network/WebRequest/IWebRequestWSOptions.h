#pragma once
#include "Common.h"
#include "Options.h"

namespace kxf
{
	class KXF_API_NETWORK IWebRequestWSOptions: public RTTI::Interface<IWebRequestWSOptions>
	{
		kxf_RTTI_DeclareIID(IWebRequestWSOptions, {0x72edead4, 0xf63f, 0x44ac, {0xaf, 0x94, 0xea, 0x92, 0x11, 0xba, 0x3b, 0x6c}});

		public:
			virtual bool SetRawMode(WebRequestOption2 option) = 0;
			virtual bool SetAutoPong(WebRequestOption2 option) = 0;
			virtual bool SetWSTimeout(TimeSpan timeout) = 0;
	};
}
