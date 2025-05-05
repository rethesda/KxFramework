#pragma once
#include "Common.h"
#include "FSPath.h"
#include "FileItem.h"
#include "LegacyVolume.h"
#include "kxf/Core/DataSize.h"
#include "kxf/Core/Enumerator.h"
#include "kxf/System/SystemWindow.h"
class wxWindow;

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
			std::shared_ptr<IFileSystem> m_FileSystem;
			SystemWindow m_Window;

		public:
			RecycleBin(LegacyVolume volume);
			RecycleBin(LegacyVolume volume, std::shared_ptr<IFileSystem> fileSystem = nullptr);

		public:
			bool IsEnabled() const;
			void SetWindow(SystemWindow window);

			DataSize GetSize() const;
			size_t GetItemCount() const;
			bool ClearItems(FlagSet<FSActionFlag> flags = {});

			FileItem GetItem(const FSPath& path) const;
			Enumerator<FileItem> EnumItems() const;

			bool Recycle(const FSPath& path, FlagSet<FSActionFlag> flags = {});
			bool Restore(const FSPath& path, FlagSet<FSActionFlag> flags = {});
	};
}
