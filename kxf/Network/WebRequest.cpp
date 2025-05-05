#include "kxf-pch.h"
#include "WebRequest.h"
#include "URI.h"
//#include "WSPPWebSession/WSPPWebSession.h"
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
	std::vector<IPAddress> LookupIP(const URI& uri, NetworkHostType ip)
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
			WORD type = ip == NetworkHostType::IPv6 ? DNS_TYPE_AAAA : DNS_TYPE_A;
			DWORD flags = DNS_QUERY_STANDARD|DNS_QUERY_NO_HOSTS_FILE|DNS_QUERY_WIRE_ONLY|DNS_QUERY_BYPASS_CACHE;
			if (::DnsQuery_W(hostName.wc_str(), type, flags, nullptr, &infoDNS, nullptr) == 0 && infoDNS)
			{
				std::vector<IPAddress> results;
				auto LookFor = [&infoDNS, &results](WORD type)
				{
					for (auto record = infoDNS; ; record = record->pNext)
					{
						if (record->wType == type)
						{
							if (record->wType == DNS_TYPE_A)
							{
								if (!results.emplace_back(IPAddress::FromIPv4(record->Data.A.IpAddress, NetworkByteOrder::Network)))
								{
									results.pop_back();
								}
							}
							else if (record->wType == DNS_TYPE_AAAA)
							{
								auto& ipv6 = record->Data.AAAA.Ip6Address;
								static_assert(sizeof(ipv6) == 16);

								std::array<uint8_t, 16> data;
								std::memcpy(data.data(), &ipv6, sizeof(ipv6));
								if (!results.emplace_back(IPAddress::FromIPv6(data)))
								{
									results.pop_back();
								}
							}
						}

						if (!record->pNext)
						{
							break;
						}
					}
				};

				if (ip == NetworkHostType::IPv4)
				{
					LookFor(DNS_TYPE_A);
				}
				else if (ip == NetworkHostType::IPv6)
				{
					LookFor(DNS_TYPE_AAAA);
				}
				return results;
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
				//session = std::make_shared<WSPPWebSession>(std::move(taskExecutor));
			}
			else
			{
				session = std::make_shared<CURLWebSession>(std::move(taskExecutor));
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
