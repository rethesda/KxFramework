#pragma once
#include "kxf/Core/String.h"

namespace kxf
{
	enum class SoftwareLicenseType
	{
		None = -1,

		MIT,
		ZLib,
		GNU_GPLv3,
		GNU_LGPLv3,
		BSD2_Clause,
		BSD3_Clause
	};
}

namespace kxf
{
	class KXF_API SoftwareLicenseDB final
	{
		public:
			static const SoftwareLicenseDB& Get();

		private:
			SoftwareLicenseDB() noexcept = default;
			SoftwareLicenseDB(const SoftwareLicenseDB&) = delete;

		public:
			String GetName(SoftwareLicenseType licenseType) const;
			String GetText(SoftwareLicenseType licenseType, const String& copyright = {}) const;

			bool RequiresCopyrightString(SoftwareLicenseType licenseType) const;

		public:
			SoftwareLicenseDB& operator=(const SoftwareLicenseDB&) = delete;
	};
}
