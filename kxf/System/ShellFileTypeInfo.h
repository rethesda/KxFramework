#pragma once
#include "Common.h"
#include "kxf/Core/CallbackFunction.h"
#include "kxf/Utility/String.h"
class wxFileTypeInfo;

namespace kxf
{
	class KXF_API ShellFileTypeInfo final
	{
		private:
			std::unique_ptr<wxFileTypeInfo> m_FileTypeInfo;
			Utility::UnorderedMapIC<String, bool> m_URLProtocolMap;

		public:
			ShellFileTypeInfo() = default;
			ShellFileTypeInfo(const String& mimeType);
			ShellFileTypeInfo(const String& mimeType,
							  const String& openCommand,
							  const String& printCommand,
							  const String& description,
							  const String& extension
			);

			ShellFileTypeInfo(const wxFileTypeInfo& other);
			ShellFileTypeInfo(const ShellFileTypeInfo&) = delete;
			ShellFileTypeInfo(ShellFileTypeInfo&&) noexcept = default;
			~ShellFileTypeInfo();

		public:
			wxFileTypeInfo& AsWXFileTypeInfo() noexcept
			{
				return *m_FileTypeInfo;
			}
			const wxFileTypeInfo& AsWXFileTypeInfo() const noexcept
			{
				return *m_FileTypeInfo;
			}

		public:
			String GetDescription() const;
			ShellFileTypeInfo& SetDescription(const String& value);
		
			String GetShortDescription() const;
			ShellFileTypeInfo& SetShortDescription(const String& value);
		
			String GetMimeType() const;

			String GetOpenCommand() const;
			ShellFileTypeInfo& SetOpenCommand(const String& value);

			String GetPrintCommand() const;
			ShellFileTypeInfo& SetPrintCommand(const String& value);

			size_t GetExtensionsCount() const;
			CallbackResult<size_t> EnumExtensions(CallbackFunction<String> func) const;
			ShellFileTypeInfo& AddExtension(const String& extension, bool isURLProtocol = false);
			
			template<class... Args>
			ShellFileTypeInfo& AddExtensions(Args&&... arg)
			{
				if (m_FileTypeInfo)
				{
					Utility::ForEachParameterPackItem([this](const String& extension)
					{
						AddExtension(extension);
					}, std::forward<Args>(arg)...);
				}
				return *this;
			}

			int GetIconIndex() const;
			String GetIconFile() const;
			ShellFileTypeInfo& SetIcon(const String& filePath, int index = 0);

			bool IsURLProtocol(const String& extension) const;
			ShellFileTypeInfo& SetURLProtocol(const String& extension, bool protocol = true);

		public:
			explicit operator bool() const noexcept
			{
				return m_FileTypeInfo != nullptr;
			}
			bool operator!() const noexcept
			{
				return m_FileTypeInfo == nullptr;
			}

			ShellFileTypeInfo& operator=(const ShellFileTypeInfo&) = delete;
			ShellFileTypeInfo& operator=(ShellFileTypeInfo&&) = default;
	};
}
