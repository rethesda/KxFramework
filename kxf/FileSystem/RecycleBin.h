#pragma once
#include "Common.h"
#include "FSPath.h"
#include "FileItem.h"
#include "LegacyVolume.h"
#include "kxf/General/BinarySize.h"
#include "kxf/General/Enumerator.h"

namespace kxf
{
	class IFileSystem;
}

namespace kxf
{
	class RecycleBin final
	{
		private:
			XChar m_Path[4] = {};
			LegacyVolume m_Volume;
			IFileSystem* m_FileSystem = nullptr;
			wxWindow* m_Window = nullptr;

		public:
			RecycleBin(LegacyVolume volume = {});
			RecycleBin(LegacyVolume volume, IFileSystem& fileSystem);

		public:
			bool IsEnabled() const;
			void SetWindow(wxWindow* window);

			BinarySize GetSize() const;
			size_t GetItemCount() const;
			bool ClearItems(FlagSet<FSActionFlag> flags = {});

			FileItem GetItem(const FSPath& path) const;
			Enumerator<FileItem> EnumItems() const;

			bool Recycle(const FSPath& path, FlagSet<FSActionFlag> flags = {});
			bool Restore(const FSPath& path, FlagSet<FSActionFlag> flags = {});
	};
}
