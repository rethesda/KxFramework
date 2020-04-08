#pragma once
#include "Common.h"
#include "NativeFileSystem.h"
#include "Kx/Utility/Common.h"

namespace KxFramework::FileSystem::NativeUtility
{
	inline HANDLE CallFindFirstFile(const wxString& query, WIN32_FIND_DATAW& findInfo, bool isCaseSensitive = false)
	{
		const DWORD searchFlags = FIND_FIRST_EX_LARGE_FETCH|(isCaseSensitive ? FIND_FIRST_EX_CASE_SENSITIVE : 0);
		return ::FindFirstFileExW(query.wc_str(), FindExInfoBasic, &findInfo, FINDEX_SEARCH_OPS::FindExSearchNameMatch, nullptr, searchFlags);
	}

	inline FileAttribute MapFileAttributes(uint32_t nativeAttributes)
	{
		if (nativeAttributes == INVALID_FILE_ATTRIBUTES)
		{
			return FileAttribute::Invalid;
		}
		else if (nativeAttributes == FILE_ATTRIBUTE_NORMAL)
		{
			return FileAttribute::Normal;
		}
		else
		{
			FileAttribute attributes = FileAttribute::None;
			Utility::ModFlagRef(attributes, FileAttribute::Hidden, nativeAttributes & FILE_ATTRIBUTE_HIDDEN);
			Utility::ModFlagRef(attributes, FileAttribute::Archive, nativeAttributes & FILE_ATTRIBUTE_ARCHIVE);
			Utility::ModFlagRef(attributes, FileAttribute::Directory, nativeAttributes & FILE_ATTRIBUTE_DIRECTORY);
			Utility::ModFlagRef(attributes, FileAttribute::ReadOnly, nativeAttributes & FILE_ATTRIBUTE_READONLY);
			Utility::ModFlagRef(attributes, FileAttribute::System, nativeAttributes & FILE_ATTRIBUTE_SYSTEM);
			Utility::ModFlagRef(attributes, FileAttribute::Temporary, nativeAttributes & FILE_ATTRIBUTE_TEMPORARY);
			Utility::ModFlagRef(attributes, FileAttribute::Compressed, nativeAttributes & FILE_ATTRIBUTE_COMPRESSED);
			Utility::ModFlagRef(attributes, FileAttribute::Encrypted, nativeAttributes & FILE_ATTRIBUTE_ENCRYPTED);
			Utility::ModFlagRef(attributes, FileAttribute::ReparsePoint, nativeAttributes & FILE_ATTRIBUTE_REPARSE_POINT);
			Utility::ModFlagRef(attributes, FileAttribute::SparseFile, nativeAttributes & FILE_ATTRIBUTE_SPARSE_FILE);
			Utility::ModFlagRef(attributes, FileAttribute::Offline, nativeAttributes & FILE_ATTRIBUTE_OFFLINE);
			Utility::ModFlagRef(attributes, FileAttribute::ContentIndexed, !(nativeAttributes & FILE_ATTRIBUTE_NOT_CONTENT_INDEXED));
			Utility::ModFlagRef(attributes, FileAttribute::RecallOnOpen, nativeAttributes & FILE_ATTRIBUTE_RECALL_ON_OPEN);
			Utility::ModFlagRef(attributes, FileAttribute::RecallOnDataAccess, nativeAttributes & FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS);

			return attributes;
		}
	}
	inline uint32_t MapFileAttributes(FileAttribute attributes)
	{
		if (attributes == FileAttribute::Invalid)
		{
			return INVALID_FILE_ATTRIBUTES;
		}
		else if (attributes == FileAttribute::Normal)
		{
			return FILE_ATTRIBUTE_NORMAL;
		}
		else
		{
			int32_t nativeAttributes = 0;
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_HIDDEN, attributes & FileAttribute::Hidden);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_ARCHIVE, attributes & FileAttribute::Archive);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_DIRECTORY, attributes & FileAttribute::Directory);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_READONLY, attributes & FileAttribute::ReadOnly);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_SYSTEM, attributes & FileAttribute::System);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_TEMPORARY, attributes & FileAttribute::Temporary);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_COMPRESSED, attributes & FileAttribute::Compressed);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_ENCRYPTED, attributes & FileAttribute::Encrypted);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_REPARSE_POINT, attributes & FileAttribute::ReparsePoint);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_SPARSE_FILE, attributes & FileAttribute::SparseFile);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_OFFLINE, attributes & FileAttribute::Offline);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_NOT_CONTENT_INDEXED, !(attributes & FileAttribute::ContentIndexed));
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_RECALL_ON_OPEN, attributes & FileAttribute::RecallOnOpen);
			Utility::ModFlagRef(nativeAttributes, FILE_ATTRIBUTE_RECALL_ON_DATA_ACCESS, attributes & FileAttribute::RecallOnDataAccess);

			return nativeAttributes;
		}
	}

	inline ReparsePointTag MapReparsePointTags(uint32_t nativeTags)
	{
		// https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-fscc/c8e77b37-3909-4fe6-a4ea-2b9d423b1ee4
		using namespace KxFramework;

		ReparsePointTag tags = ReparsePointTag::None;
		Utility::ModFlagRef(tags, ReparsePointTag::MountPoint, nativeTags & IO_REPARSE_TAG_MOUNT_POINT);
		Utility::ModFlagRef(tags, ReparsePointTag::SymLink, nativeTags & IO_REPARSE_TAG_SYMLINK);

		return tags;
	}
	inline wxDateTime ConvertDateTime(const FILETIME& fileTime)
	{
		if (fileTime.dwHighDateTime != 0 && fileTime.dwLowDateTime != 0)
		{
			SYSTEMTIME systemTime = {0};
			SYSTEMTIME localTime = {0};
			if (::FileTimeToSystemTime(&fileTime, &systemTime) && ::SystemTimeToTzSpecificLocalTime(nullptr, &systemTime, &localTime))
			{
				return wxDateTime().SetFromMSWSysTime(localTime);
			}
		}
		return wxInvalidDateTime;
	}
	inline FileItem ConvertFileInfo(const WIN32_FIND_DATAW& findInfo, const FSPath& location)
	{
		using namespace KxFramework;

		FileItem fileItem;

		// Construct path
		FSPath fsPath(findInfo.cFileName);
		fsPath.EnsureNamespaceSet(location.GetNamespace());

		// Attributes and reparse point
		fileItem.SetAttributes(MapFileAttributes(findInfo.dwFileAttributes));
		if (findInfo.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT)
		{
			fileItem.SetReparsePointTags(MapReparsePointTags(findInfo.dwReserved0));
		}

		// File size
		if (!(findInfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			ULARGE_INTEGER size = {0};
			size.HighPart = findInfo.nFileSizeHigh;
			size.LowPart = findInfo.nFileSizeLow;

			fileItem.SetSize(BinarySize::FromBytes(size.QuadPart));
		}

		// Compressed file size
		if (findInfo.dwFileAttributes & FILE_ATTRIBUTE_COMPRESSED)
		{
			ULARGE_INTEGER compressedSize = {};

			wxString path = fsPath.GetFullPathWithNS();
			compressedSize.LowPart = ::GetCompressedFileSizeW(path.wc_str(), &compressedSize.HighPart);
			fileItem.SetCompressedSize(BinarySize::FromBytes(compressedSize.QuadPart));
		}

		// Date and time
		fileItem.SetCreationTime(ConvertDateTime(findInfo.ftCreationTime));
		fileItem.SetModificationTime(ConvertDateTime(findInfo.ftLastWriteTime));
		fileItem.SetLastAccessTime(ConvertDateTime(findInfo.ftLastAccessTime));

		// Assign path
		fileItem.SetFullPath(std::move(fsPath));

		return fileItem;
	}
	inline bool IsValidFindItem(const WIN32_FIND_DATAW& findInfo)
	{
		std::wstring_view name = findInfo.cFileName;
		return !(findInfo.dwFileAttributes == INVALID_FILE_ATTRIBUTES || name.empty() || name == L".." || name == L".");
	}

	inline DWORD CALLBACK CopyCallback(LARGE_INTEGER TotalFileSize,
									   LARGE_INTEGER TotalBytesTransferred,
									   LARGE_INTEGER StreamSize,
									   LARGE_INTEGER StreamBytesTransferred,
									   DWORD dwStreamNumber,
									   DWORD dwCallbackReason,
									   HANDLE hSourceFile,
									   HANDLE hDestinationFile,
									   LPVOID lpData)
	{
		IFileSystem::TCopyItemFunc& func = *reinterpret_cast<IFileSystem::TCopyItemFunc*>(lpData);
		if (func == nullptr || std::invoke(func, BinarySize::FromBytes(TotalBytesTransferred.QuadPart), BinarySize::FromBytes(TotalFileSize.QuadPart)))
		{
			return PROGRESS_CONTINUE;
		}
		return PROGRESS_CANCEL;
	}

	inline bool CopyOrMoveDirectoryTree(NativeFileSystem& fileSystem,
										const FSPath& source,
										const FSPath& destination,
										NativeFileSystem::TCopyDirectoryTreeFunc func,
										FSCopyItemFlag flags, bool move)
	{
		return fileSystem.EnumItems(source, [&](FileItem item)
		{
			FSPath target = destination / item.GetFullPath().GetAfter(source);
			if (item.IsDirectory())
			{
				if (!func || std::invoke(func, source, target, 0, 0))
				{
					fileSystem.CreateDirectory(target);
					if (move)
					{
						fileSystem.RemoveItem(source);
					}
				}
				else
				{
					return false;
				}
			}
			else
			{
				if (func)
				{
					auto ForwardCallback = [&](BinarySize copied, BinarySize total)
					{
						return std::invoke(func, source, std::move(target), copied, total);
					};
					return move ? fileSystem.MoveItem(source, target, std::move(ForwardCallback), flags) : fileSystem.CopyItem(source, target, std::move(ForwardCallback), flags);
				}
				else
				{
					return move ? fileSystem.MoveItem(source, target, {}, flags) : fileSystem.CopyItem(source, target, {}, flags);
				}
			}
			return true;
		}, {}, FSEnumItemsFlag::Recursive) != 0;
	}
}
