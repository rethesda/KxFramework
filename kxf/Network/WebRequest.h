#pragma once
#include "Common.h"
#include "IPAddress.h"

#include "WebRequest/IWebSession.h"
#include "WebRequest/IWebRequest.h"
#include "WebRequest/IWebResponse.h"
#include "WebRequest/IWebAuthChallenge.h"
#include "WebRequest/IWebRequestOptions.h"
#include "WebRequest/IWebRequestAuthOptions.h"
#include "WebRequest/IWebRequestProxyOptions.h"
#include "WebRequest/IWebRequestSecurityOptions.h"
#include "WebRequest/WebRequestEvent.h"
#include "WebRequest/WebRequestHeader.h"

namespace kxf
{
	class IAsyncTaskExecutor;
}

namespace kxf::Network
{
	KXF_API_NETWORK bool IsInternetAvailable() noexcept;
	KXF_API_NETWORK std::vector<IPAddress> LookupIP(const URI& uri, NetworkHostType ip);

	KXF_API_NETWORK std::shared_ptr<IWebSession> CreateWebSession(const URI& uri, std::shared_ptr<IAsyncTaskExecutor> taskExecutor = {});
}
