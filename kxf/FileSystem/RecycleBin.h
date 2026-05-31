#pragma once
#include "Common.h"
#include "FSPath.h"
#include "FileItem.h"
#include "LegacyVolume.h"
#include "kxf/Core/DataSize.h"
#include "kxf/Core/CallbackFunction.h"
#include "kxf/System/SystemWindow.h"

namespace kxf
{
	class IFileSystem;
}

namespace kxf
{
	class KXF_API RecycleBin final
	{
		private:
			XChar m_Path[4] = {};
			LegacyVolume m_Volume;
			SystemWindow m_HostWindow;
			std::shared_ptr<IFileSystem> m_FileSystem;

		public:
			RecycleBin(LegacyVolume volume, std::shared_ptr<IFileSystem> fileSystem = nullptr);

		public:
			bool IsEnabled() const;
			void SetHostWindow(SystemWindow window);

			DataSize GetSize() const;
			size_t GetItemCount() const;
			bool ClearItems(FlagSet<FSActionFlag> flags = {});

			FileItem GetItem(const FSPath& path) const;
			CallbackResult<void> EnumItems(CallbackFunction<FileItem> func) const;

			bool Recycle(const FSPath& path, FlagSet<FSActionFlag> flags = {});
			bool Restore(const FSPath& path, FlagSet<FSActionFlag> flags = {});
	};
}
