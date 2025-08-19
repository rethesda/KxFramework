#pragma once
#include "Common.h"
#include "IWebRequest.h"
#include "IWebResponse.h"
#include "WebRequestHeader.h"
#include "kxf/EventSystem/Event.h"

namespace kxf
{
	class KXF_API_NETWORK WebRequestEvent: public BasicEvent
	{
		public:
			kxf_EVENT_MEMBER(WebRequestEvent, StateChanged);
			kxf_EVENT_MEMBER(WebRequestEvent, HeaderReceived);

			kxf_EVENT_MEMBER(WebRequestEvent, DataSent);
			kxf_EVENT_MEMBER(WebRequestEvent, DataReceived);
			kxf_EVENT_MEMBER(WebRequestEvent, DataProgress);

		protected:
			std::shared_ptr<IWebRequest> m_Request;
			IWebResponse* m_Response = nullptr;
			WebRequestState m_State = WebRequestState::None;

			WebRequestHeader m_Header;
			const void* m_Buffer = nullptr;
			size_t m_BufferSize = 0;

		public:
			WebRequestEvent(std::shared_ptr<IWebRequest> request, WebRequestState state)
				:m_Request(std::move(request)), m_State(state)
			{
			}
			WebRequestEvent(std::shared_ptr<IWebRequest> request, IWebResponse& response, WebRequestState state)
				:m_Request(std::move(request)), m_Response(&response), m_State(state)
			{
			}

		public:
			// IEvent
			std::unique_ptr<IEvent> Move() noexcept override
			{
				return std::make_unique<WebRequestEvent>(std::move(*this));
			}

			// WebRequestEvent
			IWebRequest& GetRequest() const
			{
				return *m_Request;
			}
			IWebResponse& GetResponse() const
			{
				return m_Response ? *m_Response : NullWebResponse::Get();
			}
			WebRequestState GetState() const
			{
				return m_State;
			}

			const WebRequestHeader& GetHeader() const&
			{
				return m_Header;
			}
			WebRequestHeader GetHeader() &&
			{
				return std::move(m_Header);
			}
			void SetHeader(WebRequestHeader header)
			{
				m_Header = std::move(header);
			}

			const void* GetBuffer() const
			{
				return m_Buffer;
			}
			size_t GetBufferSize() const
			{
				return m_BufferSize;
			}
			void SetBuffer(const void* ptr, size_t size)
			{
				m_Buffer = ptr;
				m_BufferSize = size;
			}
	};
}

namespace kxf
{
	class KXF_API_NETWORK WebRequestWSEvent: public WebRequestEvent
	{
		public:
			kxf_EVENT_MEMBER(WebRequestWSEvent, WebSocketOpen);
			kxf_EVENT_MEMBER(WebRequestWSEvent, WebSocketClose);
			kxf_EVENT_MEMBER(WebRequestWSEvent, WebSocketPing);
			kxf_EVENT_MEMBER(WebRequestWSEvent, WebSocketMessage);

		private:
			String m_Payload;

		public:
			WebRequestWSEvent(std::shared_ptr<IWebRequest> request, IWebResponse& response, WebRequestState state)
				:WebRequestEvent(std::move(request), response, state)
			{
			}

		public:
			// IEvent
			std::unique_ptr<IEvent> Move() noexcept override
			{
				return std::make_unique<WebRequestWSEvent>(std::move(*this));
			}

			// WebRequestWSEvent
			const String& GetPayload() const&
			{
				return m_Payload;
			}
			String GetPayload() &&
			{
				return std::move(m_Payload);
			}
			void SetPayload(String payload)
			{
				m_Payload = std::move(payload);
			}
	};
}
