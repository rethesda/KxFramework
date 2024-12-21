#pragma once
#include "Common.h"
class wxFileType;

namespace kxf
{
	class ShellFileTypeInfo;
}

namespace kxf
{
	class KXF_API ShellFileType final
	{
		public:
			class MessageParameters final
			{
				friend class ShellFileType;

				private:
					String m_FileName;
					String m_MIMEType;

				public:
					MessageParameters() = default;
					MessageParameters(String fileName, String mimeType = {})
						:m_FileName(std::move(fileName)), m_MIMEType(std::move(mimeType))
					{
					}

				public:
					const String& GetFileName() const noexcept
					{
						return m_FileName;
					}
					const String& GetMIMEType() const noexcept
					{
						return m_MIMEType;
					}
			};

			class IconLocation final
			{
				friend class ShellFileType;

				private:
					String m_FileName;
					int m_Index = -1;

				public:
					IconLocation() = default;
					IconLocation(String fileName, int index = -1)
						:m_FileName(std::move(fileName)), m_Index(index)
					{
					}

				public:
					const String& GetFileName() const noexcept
					{
						return m_FileName;
					}
					int GetIndex() const noexcept
					{
						return m_Index;
					}
			};

		public:
			static String ExpandCommand(const String &command, const MessageParameters& parameters);

		private:
			std::unique_ptr<wxFileType> m_FileType;

		public:
			ShellFileType() = default;
			ShellFileType(wxFileType* fileType);
			ShellFileType(const ShellFileTypeInfo& typeInfo);
			ShellFileType(ShellFileType&&) noexcept = default;
			ShellFileType(const ShellFileType&) = delete;
			~ShellFileType();

		public:
			wxFileType& AsWXFileType()
			{
				return *m_FileType;
			}
			const wxFileType& AsWXFileType() const
			{
				return *m_FileType;
			}

		public:
			String GetDescription() const;
			String GetExtension() const;
			std::vector<String> GetAllExtensions() const;

			String GetMimeType() const;
			std::vector<String> GetAllMimeTypes() const;

			IconLocation GetIcon() const;
			IconLocation GetIcon(const MessageParameters& parameters) const;

			String GetCommand(const String& action, const String& filePath) const;
			String GetCommand(const String& action, const MessageParameters& parameters) const;

			String GetOpenCommand(const String& filePath) const;
			String GetOpenCommand(const MessageParameters& parameters) const;
			String GetOpenExecutable() const;

			String GetPrintCommand(const String& filePath) const;
			String GetPrintCommand(const MessageParameters& parameters) const;
		
			size_t GetAllCommands(std::vector<String>& verbs, std::vector<String>& commands, const MessageParameters& parameters) const;
			bool IsURLProtocol(const String& extension) const;

		public:
			explicit operator bool() const noexcept
			{
				return m_FileType != nullptr;
			}
			bool operator!() const noexcept
			{
				return m_FileType == nullptr;
			}

			ShellFileType& operator=(ShellFileType&&) noexcept = default;
			ShellFileType& operator=(const ShellFileType&) = delete;
	};
}
