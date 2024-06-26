// This file is based on the following file from the LZMA SDK (http://www.7-zip.org/sdk.html):
// ./CPP/7zip/Guid.txt
#pragma once
#include "kxf/Core/NativeUUID.h"

namespace kxf::SevenZip::GUID
{
	// IStream.h

	// {23170F69-40C1-278A-0000-000300010000}
	constexpr NativeUUID IID_ISequentialInStream = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000300010000}
	constexpr NativeUUID IID_ISequentialOutStream = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x02, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000300030000}
	constexpr NativeUUID IID_IInStream = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000300040000}
	constexpr NativeUUID IID_IOutStream = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x04, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000300060000}
	constexpr NativeUUID IID_IStreamGetSize = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00};

	// ICoder.h

	// {23170F69-40C1-278A-0000-000400040000}
	constexpr NativeUUID IID_ICompressProgressInfo = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x04, 0x00, 0x04, 0x00, 0x00};

	// IPassword.h

	// {23170F69-40C1-278A-0000-000500100000}
	constexpr NativeUUID IID_ICryptoGetTextPassword = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x10, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000500110000}
	constexpr NativeUUID IID_ICryptoGetTextPassword2 = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x05, 0x00, 0x11, 0x00, 0x00};

	// IArchive.h

	// {23170F69-40C1-278A-0000-000600030000}
	constexpr NativeUUID IID_ISetProperties = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x03, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000600100000}
	constexpr NativeUUID IID_IArchiveOpenCallback = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x10, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000600200000}
	constexpr NativeUUID IID_IArchiveExtractCallback = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x20, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000600600000}
	constexpr NativeUUID IID_IInArchive = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x60, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000600800000}
	constexpr NativeUUID IID_IArchiveUpdateCallback = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x80, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000600820000}
	constexpr NativeUUID IID_IArchiveUpdateCallback2 = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0x82, 0x00, 0x00};

	// {23170F69-40C1-278A-0000-000600A00000}
	constexpr NativeUUID IID_IOutArchive = {0x23170F69, 0x40C1, 0x278A, 0x00, 0x00, 0x00, 0x06, 0x00, 0xA0, 0x00, 0x00};

	// Handler GUIDs

	// {23170F69-40C1-278A-1000-000110010000}
	constexpr NativeUUID CLSID_CFormatZip = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-000110020000}
	constexpr NativeUUID CLSID_CFormatBZip2 = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x02, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-0001100CC000}
	constexpr NativeUUID CLSID_CFormatRar = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-000110030000}
	constexpr NativeUUID CLSID_CFormatRar5 = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xCC, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-000110070000}
	constexpr NativeUUID CLSID_CFormat7z = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-000110080000}
	constexpr NativeUUID CLSID_CFormatCab = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x08, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-0001100A0000}
	constexpr NativeUUID CLSID_CFormatLzma = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0A, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-0001100B0000}
	constexpr NativeUUID CLSID_CFormatLzma86 = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0B, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-000110E70000}
	constexpr NativeUUID CLSID_CFormatIso = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE7, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-000110EE0000}
	constexpr NativeUUID CLSID_CFormatTar = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEE, 0x00, 0x00};

	// {23170F69-40C1-278A-1000-000110EF0000}
	constexpr NativeUUID CLSID_CFormatGZip = {0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEF, 0x00, 0x00};
}
