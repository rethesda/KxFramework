#pragma once
#include "../Common.h"
#include "kxf/Core/String.h"
#include "kxf/Core/Version.h"
#include "kxf/Core/NativeUUID.h"
#include "kxf/Core/UniversallyUniqueID.h"
#include "kxf/EventSystem/IEvtHandler.h"
#include "kxf/EventSystem/EvtHandlerDelegate.h"
#include "kxf/FileSystem/IFileSystem.h"
#include "kxf/FileSystem/FSPath.h"
#include "kxf/FileSystem/FileItem.h"
#include "kxf/Compression/IArchive.h"
#include "kxf/Compression/ArchiveEvent.h"

namespace kxf::SevenZip
{
	enum class CompressionMethod
	{
		Unknown = -1,

		LZMA,
		LZMA2,
		PPMD,
		BZIP2,
	};
	enum class CompressionFormat
	{
		Unknown = -1,

		SevenZip,
		Zip,
		GZip,
		BZip2,
		Rar,
		Rar5,
		Tar,
		Iso,
		Cab,
		Lzma,
		Lzma86
	};
	enum class CompressionLevel
	{
		None = 0,

		Fastest = 1,
		Fast = 2,
		Normal = 5,
		Maximum = 7,
		Ultra = 9,
	};
}

namespace kxf::SevenZip
{
	String GetLibraryName();
	Version GetLibraryVersion();

	NativeUUID GetAlgorithmID(CompressionFormat format) noexcept;
	String GetNameByFormat(CompressionFormat format);
	String GetExtensionByFormat(CompressionFormat format);
	CompressionFormat GetFormatByExtension(const String& extension);
}
