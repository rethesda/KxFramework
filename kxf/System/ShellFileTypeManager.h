#pragma once
#include "Common.h"
#include "ShellFileType.h"
#include "ShellFileTypeInfo.h"
#include "kxf/Core/CallbackFunction.h"
class wxMimeTypesManager;

namespace kxf
{
	class KXF_API ShellFileTypeManager
	{
		public:
			static bool IsOfType(const String& mimeType, const String& wildcard);

		private:
			std::unique_ptr<wxMimeTypesManager> m_Manager;

		public:
			ShellFileTypeManager();
			~ShellFileTypeManager();

		public:
			ShellFileType FileTypeFromExtension(const String& extension) const;
			ShellFileType FileTypeFromMimeType(const String& mimeType) const;

			ShellFileType Associate(const ShellFileTypeInfo& fileTypeInfo);
			bool IsAssociatedWith(const ShellFileType& fileType, const String& executablePath) const;
			bool Unassociate(ShellFileType& fileType);
			void AddFallback(const ShellFileTypeInfo& fileTypeInfo);

			CallbackResult<size_t> EnumFileTypes(CallbackFunction<String> func) const;
	};
}
