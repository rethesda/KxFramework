#pragma once
#include "../Common.h"

namespace kxf
{
	class IErrorCode;
}

namespace kxf::Private
{
	KXF_API String FormatErrorCode(const IErrorCode& error);
}
