#include "kxf-pch.h"
#include "WebRequest.h"
#include "URI.h"
#include "WSPPWebSession/WSPPWebSession.h"
#include "CURLWebSession/CURLWebSession.h"
#include "kxf/Utility/ScopeGuard.h"

#include "kxf/Win32/Include-Network.h"
#include "kxf/Win32/LinkLibs-Network.h"
#include "kxf/Win32/LinkLibs-Crypto.h"
#include "kxf/Win32/UndefMacros.h"

namespace kxf::Network
{
	bool IsInternetAvailable() noexcept
	{
		DWORD flags = 0;
		return ::InternetGetConnectedState(&flags, 0);
	}
	String LookupIP(const URI& uri, NetworkHostType ip)
	{
		if (ip == NetworkHostType::IPv4 || ip == NetworkHostType::IPv6)
		{
			DNS_RECORD* infoDNS = nullptr;
			Utility::ScopeGuard atExit([&]()
			{
				if (infoDNS)
				{
					DnsRecordListFree(infoDNS, DnsFreeRecordListDeep);
				}
			});

			String hostName = uri.GetServer();
			WORD type = ip == NetworkHostType::IPv6 ? DNS_TYPE_A6 : DNS_TYPE_A;
			DWORD flags = DNS_QUERY_STANDARD|DNS_QUERY_BYPASS_CACHE;
			if (::DnsQuery_W(hostName.wc_str(), type, flags, nullptr, &infoDNS, nullptr) == 0 && infoDNS)
			{
				bool isSuccess = false;
				wchar_t buffer[64] = {};

				if (ip == NetworkHostType::IPv6)
				{
					isSuccess = ::InetNtopW(AF_INET6, &infoDNS->Data.AAAA.Ip6Address, buffer, std::size(buffer)) != nullptr;
				}
				else
				{
					isSuccess = ::InetNtopW(AF_INET, &infoDNS->Data.A.IpAddress, buffer, std::size(buffer)) != nullptr;
				}

				if (isSuccess)
				{
					return buffer;
				}
			}
		}
		return {};
	}

	std::shared_ptr<IWebSession> CreateWebSession(const URI& uri, std::shared_ptr<IAsyncTaskExecutor> taskExecutor)
	{
		if (uri)
		{
			std::shared_ptr<IWebSession> session;

			const auto scheme = uri.GetScheme();
			if (scheme.IsSameAs("ws", StringActionFlag::IgnoreCase) || scheme.IsSameAs("wss", StringActionFlag::IgnoreCase))
			{
				session = std::make_unique<WSPPWebSession>(std::move(taskExecutor));
			}
			else
			{
				session = std::make_unique<CURLWebSession>(std::move(taskExecutor));
			}

			if (session)
			{
				session->SetBaseURI(uri);
			}
			return session;
		}
		return nullptr;
	}
}
