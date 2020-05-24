#include "stdafx.h"
#include "Common.h"
#include "SciterAPI.h"
#include "Internal.h"
#include "Kx/System/DynamicLibrary.h"
#include "Kx/System/ExecutableVersionResource.h"

namespace kxf::Sciter
{
	void* g_SciterLibrary = nullptr;

	String GetLibraryName()
	{
		return wxS("Sciter");
	}
	Version GetLibraryVersion()
	{
		if (g_SciterLibrary)
		{
			DynamicLibrary library;
			library.AttachHandle(g_SciterLibrary);

			ExecutableVersionResource versionResource(library.GetFilePath());
			if (versionResource)
			{
				return versionResource.GetAnyVersion();
			}
		}
		return {};
	}

	bool IsLibraryLoaded()
	{
		return g_SciterLibrary != nullptr;
	}
	bool LoadLibrary(const String& path)
	{
		if (!g_SciterLibrary)
		{
			g_SciterLibrary = ::LoadLibraryW(path.wc_str());
		}
		return g_SciterLibrary != nullptr;
	}
	void FreeLibrary()
	{
		// Sciter API included don't have function to unload its DLL and it causes a crash
		// if I do unload the DLL. So I'm going to let it loaded. It seems Sciter does something
		// in its DllMain's 'DLL_PROCESS_DETACH' event.
		#if 0
		if (g_SciterLibrary)
		{
			::FreeLibrary(reinterpret_cast<HMODULE>(g_SciterLibrary));
			g_SciterLibrary = nullptr;
		}
		#endif
	}
}

namespace kxf::Sciter
{
	String SizeUnitToString(SizeUnit unit)
	{
		#define Map(unit)	\
			case SizeUnit::##unit:	\
			{	\
				return wxS(#unit);	\
			}	\

		switch (unit)
		{
			// Absolute
			Map(cm);
			Map(mm);
			Map(in);
			Map(px);
			Map(pt);
			Map(pc);
			Map(dip);

			// Relative
			Map(em);
			Map(ex);
			Map(ch);
			Map(rem);
			Map(vw);
			Map(vh);
			Map(vmin);
			Map(vmax);

			case SizeUnit::pr:
			{
				return wxS("%");
			}
			case SizeUnit::sp:
			{
				return wxS("%%");
			}
		};
		return {};

		#undef Map
	}

	CornerAlignment MapCornerAlignment(FlagSet<Alignment> alignment)
	{
		if (alignment == Alignment::Invalid)
		{
			return CornerAlignment::Unspecified;
		}
		else if (alignment & Alignment::Right)
		{
			// Right alignment
			if (alignment & Alignment::CenterVertical)
			{
				return CornerAlignment::MiddleRight;
			}
			else if (alignment & Alignment::Bottom)
			{
				return CornerAlignment::BottomRight;
			}
			else
			{
				return CornerAlignment::TopRight;
			}
		}
		else if (alignment & wxALIGN_CENTRE_HORIZONTAL)
		{
			// Horizontally-centered
			if (alignment & Alignment::CenterVertical)
			{
				return CornerAlignment::MiddleCenter;
			}
			else if (alignment & Alignment::Bottom)
			{
				return CornerAlignment::BottomCenter;
			}
			else
			{
				return CornerAlignment::TopCenter;
			}
		}
		else
		{
			// Left aligned
			if (alignment & Alignment::CenterVertical)
			{
				return CornerAlignment::MiddleLeft;
			}
			else if (alignment & Alignment::Bottom)
			{
				return CornerAlignment::BottomLeft;
			}
			else
			{
				return CornerAlignment::TopLeft;
			}
		}
		return CornerAlignment::Unspecified;
	}
	FlagSet<Alignment> MapCornerAlignment(CornerAlignment alignment)
	{
		switch (alignment)
		{
			case CornerAlignment::TopLeft:
			{
				return Alignment::Left;
			}
			case CornerAlignment::TopCenter:
			{
				return Alignment::CenterHorizontal;
			}
			case CornerAlignment::TopRight:
			{
				return Alignment::Right;
			}

			case CornerAlignment::MiddleLeft:
			{
				return Alignment::CenterVertical;
			}
			case CornerAlignment::MiddleCenter:
			{
				return Alignment::CenterVertical|Alignment::CenterHorizontal;
			}
			case CornerAlignment::MiddleRight:
			{
				return Alignment::Right|Alignment::CenterVertical;
			}

			case CornerAlignment::BottomLeft:
			{
				return Alignment::Bottom;
			}
			case CornerAlignment::BottomCenter:
			{
				return Alignment::Bottom|Alignment::CenterHorizontal;
			}
			case CornerAlignment::BottomRight:
			{
				return Alignment::Bottom|Alignment::Right;
			}
		};
		return Alignment::Invalid;
	}

	bool SetMasterCSS(const String& css)
	{
		auto utf8 = ToSciterUTF8(css);
		return GetSciterAPI()->SciterSetMasterCSS(utf8.data(), utf8.size());
	}
	bool AppendMasterCSS(const String& css)
	{
		auto utf8 = ToSciterUTF8(css);
		return GetSciterAPI()->SciterAppendMasterCSS(utf8.data(), utf8.size());
	}
}
