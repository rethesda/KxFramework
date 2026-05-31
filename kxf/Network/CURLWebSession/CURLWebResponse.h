#pragma once
#include "Common.h"
#include "kxf/IO/IStream.h"
#include "kxf/IO/MemoryStream.h"

namespace kxf
{
	class CURLWebRequest;
}
namespace kxf::CURL::Private
{
	class RequestHandle;
}

namespace kxf
{
	class KXF_API_NETWORK CURLWebResponse final: public IWebResponse
	{
		private:
			CURLWebRequest& m_Request;

			String m_StatusText;
			std::optional<int> m_StatusCode;

		private:
			CURL::Private::RequestHandle& GetRequestHandle() const noexcept;

		public:
			CURLWebResponse(CURLWebRequest& request, std::optional<int> statusCode = {}, String statusText = {})
				:m_Request(request), m_StatusText(std::move(statusText)), m_StatusCode(std::move(statusCode))
			{
			}

		public:
			// IWebResponse
			URI GetURI() const override;
			String GetMethod() const override;
			String GetPrimaryIP() const override;
			std::optional<uint16_t> GetPrimaryPort() const override;
			WebRequestProtocol GetProtocol() const override;
			WebRequestHTTPVersion GetHTTPVersion() const override;
			DataSize GetContentLength() const override;
			String GetContentType() const override;
			std::optional<int> GetResponseCode() const override;

			std::optional<int> GetStatusCode() const override
			{
				return m_StatusCode;
			}
			String GetStatusText() const override
			{
				return m_StatusText;
			}

			String GetHeader(const String& name) const override;
			CallbackResult<void> EnumHeaders(CallbackFunction<WebRequestHeader> func) const override;
			CallbackResult<void> EnumCookies(CallbackFunction<String> func) const override;

			std::shared_ptr<IInputStream> GetStream() const override;

			// CURLWebResponse
			void SetStatus(std::optional<int> statusCode = {}, String statusText = {})
			{
				m_StatusText = std::move(statusText);
				m_StatusCode = std::move(statusCode);
			}
	};
}
