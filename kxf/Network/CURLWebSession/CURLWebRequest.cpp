#include "kxf-pch.h"
#include "CURLWebRequest.h"
#include "CURLWebSession.h"
#include "LibCURLUtility.h"
#include "LibCURL.h"
#include "kxf/IO/IStream.h"
#include "kxf/IO/MemoryStream.h"
#include "kxf/Core/Enumerator.h"
#include "kxf/Utility/Common.h"
#include "kxf/Utility/Container.h"
#include "kxf/Utility/ScopeGuard.h"
#include <chrono>
#include <cstdlib>

namespace
{
	using CharTraits = std::char_traits<char>;

	void NormalizeValue(kxf::String& value)
	{
		value.TrimBoth();
		value.Replace('\r', kxf::NullString);
		value.Replace('\n', kxf::NullString);
	}
	kxf::String GetHeaderName(const char* source, size_t length)
	{
		if (source && length != 0)
		{
			if (const char* colon = CharTraits::find(source, length, ':'))
			{
				auto value = kxf::String::FromUTF8({source, static_cast<size_t>(colon - source)});
				NormalizeValue(value);

				return value;
			}
		}
		return {};
	}
	kxf::String GetHeaderValue(const char* source, size_t length)
	{
		if (source && length != 0)
		{
			if (const char* colon = CharTraits::find(source, length, ':'))
			{
				// Skip colon itself and a single space after it
				constexpr size_t offset = 2;

				auto value = kxf::String::FromUTF8({colon + offset, length - (colon - source) - offset});
				NormalizeValue(value);

				return value;
			}
		}
		return {};
	}
}

namespace kxf
{
	bool CURLWebRequest::OnCallbackCommon(bool isWrite, size_t& result)
	{
		if (m_NextState == WebRequestState::Cancelled)
		{
			m_NextState = WebRequestState::None;

			result = isWrite ? CURL_WRITEFUNC_ERROR : CURL_READFUNC_ABORT;
			return true;
		}
		else if (m_NextState == WebRequestState::Paused)
		{
			m_NextState = WebRequestState::None;
			ChangeStateAndNotify(WebRequestState::Paused);

			result = isWrite ? CURL_WRITEFUNC_PAUSE : CURL_READFUNC_PAUSE;
			return true;
		}
		return false;
	}
	size_t CURLWebRequest::OnReadData(char* data, size_t size, size_t count)
	{
		size_t result = 0;
		if (OnCallbackCommon(false, result))
		{
			return result;
		}
		else if (m_SendStream)
		{
			const size_t length = size * count;
			m_SendStream->Read(data, length);

			WebRequestEvent event(LockRef(), m_State);
			event.SetBuffer(data, length);
			NotifyEvent(WebRequestEvent::EvtDataSent, event);

			return m_SendStream->LastRead().ToBytes();
		}
		return 0;
	}
	size_t CURLWebRequest::OnWriteData(char* data, size_t size, size_t count)
	{
		size_t result = 0;
		if (OnCallbackCommon(true, result))
		{
			return result;
		}
		else if (m_ReceiveStream)
		{
			const size_t length = size * count;
			switch (m_ReceiveMode)
			{
				case WebRequestReceiveMode::Append:
				{
					m_ReceiveStream->Write(data, length);
					break;
				}
				case WebRequestReceiveMode::Overwrite:
				{
					m_ReceiveStream->RewindO();
					m_ReceiveStream->SetAllocationSize(0);
					m_ReceiveStream->SetLastWrite(0);
					m_ReceiveStream->SetLastError(StreamError::Success());

					m_ReceiveStream->Write(data, length);
					break;
				}
			};

			WebRequestEvent event(LockRef(), m_State);
			event.SetBuffer(data, length);
			NotifyEvent(WebRequestEvent::EvtDataReceived, event);

			return m_ReceiveStream->LastWrite().ToBytes();
		}
		return size * count;
	}
	size_t CURLWebRequest::OnReceiveHeader(char* data, size_t size, size_t count)
	{
		const size_t length = size * count;

		WebRequestHeader header(GetHeaderName(data, length), GetHeaderValue(data, length));
		if (CURLWebSession::SetHeader(m_ResponseHeaders, header, WebRequestHeaderFlag::Add|WebRequestHeaderFlag::CoalesceSemicolon))
		{
			WebRequestEvent event(LockRef(), m_State);
			event.SetHeader(std::move(header));
			NotifyEvent(WebRequestEvent::EvtHeaderReceived, event);
		}
		return length;
	}
	int CURLWebRequest::OnProgressNotify(int64_t bytesReceived, int64_t bytesExpectedToReceive, int64_t bytesSent, int64_t bytesExpectedToSend)
	{
		// Update sizes
		m_BytesReceived = bytesReceived;
		m_BytesExpectedToReceive = bytesExpectedToReceive;
		m_BytesSent = bytesSent;
		m_BytesExpectedToSend = bytesExpectedToSend;

		// Preallocate data for the response if possible
		if (m_ReceiveStream && bytesExpectedToReceive > 0)
		{
			m_ReceiveStream->SetAllocationSize(DataSize::FromBytes(bytesExpectedToReceive));
		}

		// Handle pause and cancellation
		if (m_NextState == WebRequestState::Resumed)
		{
			m_NextState = WebRequestState::None;
			m_Handle.Resume();

			m_State = WebRequestState::Active;
			NotifyStateChange(WebRequestState::Resumed);
		}
		else if (m_NextState == WebRequestState::Cancelled)
		{
			m_NextState = WebRequestState::None;
			return -1;
		}
		return CURL_PROGRESSFUNC_CONTINUE;
	}
	bool CURLWebRequest::OnSetAuthChallengeCredentials(WebAuthChallengeSource source, UserCredentials credentials)
	{
		// TODO: Handle other types of authorization here (TLS)

		bool restart = false;
		switch (source)
		{
			case WebAuthChallengeSource::TargetServer:
			{
				m_Handle.SetOption(CURLOPT_USERNAME, credentials.GetFullName());
				m_Handle.SetOption(CURLOPT_PASSWORD, credentials.GetSecret().ToString());

				restart = true;
				break;
			}
			case WebAuthChallengeSource::ProxyServer:
			{
				m_Handle.SetOption(CURLOPT_PROXYUSERNAME, credentials.GetFullName());
				m_Handle.SetOption(CURLOPT_PROXYPASSWORD, credentials.GetSecret().ToString());

				restart = true;
				break;
			}
		};

		// Restart the request if we know ho to handle this, otherwise it'll remain in the unauthorized state
		// (instead of failed) and request processing will terminate here.
		if (restart)
		{
			credentials.WipeSecret();
			DoPerformRequest();

			return true;
		}
		return false;
	}

	WebRequestState CURLWebRequest::HandleWebSocket()
	{
		m_WebSocketsState = WebRequestState::Active;

		WebRequestWSEvent event(LockRef(), *m_Response, WebRequestState::Active);
		NotifyEvent(WebRequestWSEvent::EvtWebSocketOpen, event);

		size_t waitCount = 0;
		TimeSpan timeStart = TimeSpan::Now();
		while (m_WebSocketsState == WebRequestState::Active)
		{
			if (std::unique_lock lock(m_WebSocketsLock); true)
			{
				using namespace std::literals::chrono_literals;
				m_WebSocketsCondition.wait_for(lock, 250ms);

				int reason = 0;
				String payload;
				switch (ReceiveWebSocket(reason, payload))
				{
					case WSFrame::Wait:
					{
						// Socket is not ready, try again
						waitCount++;

						break;
					}
					case WSFrame::Fail:
					{
						m_WebSocketsState = WebRequestState::Failed;
						return WebRequestState::Failed;
					}
					case WSFrame::Close:
					{
						m_WebSocketsState = WebRequestState::Completed;

						WebRequestWSEvent event(LockRef(), *m_Response, WebRequestState::Active);
						event.SetPayload(std::move(payload));
						NotifyEvent(WebRequestWSEvent::EvtWebSocketClose, event);

						return WebRequestState::Completed;
					}
					case WSFrame::Ping:
					{
						waitCount = 0;

						WebRequestWSEvent event(LockRef(), *m_Response, WebRequestState::Active);
						event.SetPayload(std::move(payload));
						NotifyEvent(WebRequestWSEvent::EvtWebSocketPing, event);

						break;
					}
					case WSFrame::Message:
					{
						waitCount = 0;

						WebRequestWSEvent event(LockRef(), *m_Response, WebRequestState::Active);
						event.SetPayload(std::move(payload));
						NotifyEvent(WebRequestWSEvent::EvtWebSocketMessage, event);

						break;
					}
				};

				if (m_WebSocketsTimeOut.IsPositive() && TimeSpan::Now() - timeStart > m_WebSocketsTimeOut)
				{
					m_WebSocketsState = WebRequestState::Failed;

					WebRequestWSEvent event(LockRef(), *m_Response, WebRequestState::Active);
					NotifyEvent(WebRequestWSEvent::EvtWebSocketClose, event);
					break;
				}
			}
		};
		return m_WebSocketsState;
	}
	CURLWebRequest::WSFrame CURLWebRequest::ReceiveWebSocket(int& reason, String& payload)
	{
		if (m_WebSocketsState != WebRequestState::Active)
		{
			return WSFrame::Fail;
		}

		reason = 0;
		payload.Clear();

		FlagSet<int> frameFlags;
		std::array<char, 1024> buffer = {};
		do
		{
			const struct curl_ws_frame* meta = nullptr;
			size_t read = 0;

			buffer.fill(0);
			auto status = ::curl_ws_recv(*m_Handle, buffer.data(), buffer.size(), &read, &meta);
			if (status == CURLE_OK)
			{
				frameFlags = meta->flags;
				if (frameFlags.Contains(CURLWS_PING))
				{
					payload = String::FromUTF8({buffer.data(), read});
					return WSFrame::Ping;
				}
				else if (frameFlags.Contains(CURLWS_CLOSE))
				{
					reason = static_cast<int>(Utility::CompositeInteger<uint8_t>(*std::bit_cast<uint16_t*>(buffer.data())).SwapParts().GetFull());
					payload = String::FromUTF8({buffer.data() + 2, read - 2});

					return WSFrame::Close;
				}
				else if (frameFlags.Contains(CURLWS_TEXT) || frameFlags.Contains(CURLWS_BINARY))
				{
					switch (m_ReceiveMode)
					{
						case WebRequestReceiveMode::Append:
						{
							m_ReceiveStream->Write(buffer.data(), read);
							break;
						}
						case WebRequestReceiveMode::Overwrite:
						{
							// We should only overwrite the data if we've got the full message content
							if (!frameFlags.Contains(CURLWS_CONT))
							{
								m_ReceiveStream->RewindO();
								m_ReceiveStream->SetAllocationSize(0);
								m_ReceiveStream->SetLastWrite(0);
								m_ReceiveStream->SetLastError(StreamError::Success());
							}

							m_ReceiveStream->Write(buffer.data(), read);
							break;
						}
					};

					// Send full small text fragments directly into the payload
					if (!frameFlags.Contains(CURLWS_CONT) && frameFlags.Contains(CURLWS_TEXT))
					{
						payload = String::FromUTF8({buffer.data(), read});
					}

					if (!frameFlags.Contains(CURLWS_CONT))
					{
						return WSFrame::Message;
					}
					// And continue to the next fragment
				}
			}
			else if (status == CURLE_AGAIN || status == CURLE_GOT_NOTHING)
			{
				return WSFrame::Wait;
			}
			else
			{
				return WSFrame::Fail;
			}
		}
		while (frameFlags.Contains(CURLWS_CONT));

		// We shouldn't get here, but in case we did
		return WSFrame::Fail;
	}

	void CURLWebRequest::UpdateStatusText(int code)
	{
		m_StatusCode = code;

		if (strnlen(m_StatusTextBuffer.data(), m_StatusTextBuffer.size()) == 0)
		{
			m_StatusTextBuffer.fill(0);

			auto status = CURL::Private::EasyErrorCodeToString(code);
			std::copy_n(status.data(), std::min(status.size(), m_StatusTextBuffer.size()), m_StatusTextBuffer.data());
		}

		m_Response->SetStatus(m_StatusCode, m_StatusTextBuffer.data());
	}

	void CURLWebRequest::DoFreeRequestHeaders()
	{
		if (m_RequestHeadersSList)
		{
			m_Handle.SetOption(CURLOPT_HTTPHEADER, nullptr);

			::curl_slist_free_all(reinterpret_cast<curl_slist*>(m_RequestHeadersSList));
			m_RequestHeadersSList = nullptr;
		}
	}
	void CURLWebRequest::DoSetRequestHeaders()
	{
		// Reset and free any previously set headers
		DoFreeRequestHeaders();

		// Set headers
		for (const WebRequestHeader& header: m_RequestHeaders)
		{
			auto headerString = header.Format().ToUTF8();
			m_RequestHeadersSList = ::curl_slist_append(reinterpret_cast<curl_slist*>(m_RequestHeadersSList), headerString.c_str());
		}
		m_Handle.SetOption(CURLOPT_HTTPHEADER, m_RequestHeadersSList);
	}
	void CURLWebRequest::DoPrepareSendData()
	{
		if (m_SendStorage != WebRequestStorage::None)
		{
			if (m_Method.IsEmpty() || m_Method.IsSameAs("POST", StringActionFlag::IgnoreCase))
			{
				// Make sure to use read callback
				m_Handle.SetOption(CURLOPT_POSTFIELDSIZE, 0);
				m_Handle.SetOption(CURLOPT_POSTFIELDS, nullptr);

				// Switch to post
				m_Handle.SetOption(CURLOPT_POST, true);
			}
			else if (m_Method.IsSameAs("PUT", StringActionFlag::IgnoreCase))
			{
				m_Handle.SetOption(CURLOPT_INFILESIZE_LARGE, m_SendStream->GetSize().ToBytes());
				m_Handle.SetOption(CURLOPT_UPLOAD, true);
			}
		}

		if (m_Method.IsSameAs("HEAD", StringActionFlag::IgnoreCase))
		{
			m_Handle.SetOption(CURLOPT_NOBODY, true);
		}
		if (m_Method.IsSameAs("GET", StringActionFlag::IgnoreCase))
		{
			m_Handle.SetOption(CURLOPT_HTTPGET, true);
		}
		else if (!m_Method.IsEmpty())
		{
			m_Handle.SetOption(CURLOPT_CUSTOMREQUEST, m_Method);
		}
	}
	void CURLWebRequest::DoPrepareReceiveData()
	{
		if (m_ReceiveStorage == WebRequestStorage::Memory)
		{
			m_ReceiveStream = std::make_shared<MemoryOutputStream>();
		}
		if (m_ReceiveMode == WebRequestReceiveMode::Default)
		{
			m_ReceiveMode = WebRequestReceiveMode::Append;
		}
	}
	void CURLWebRequest::DoPerformRequest()
	{
		Utility::ScopeGuard atExit = [&]()
		{
			m_Handle.SetOption(CURLOPT_ERRORBUFFER, nullptr);
		};

		// Prepare the request
		DoResetState();
		DoPrepareSendData();
		DoPrepareReceiveData();

		// Activate the request and notify about it
		ChangeStateAndNotify(WebRequestState::Active);

		// And start the request
		if (auto scheme = m_URI.GetScheme(); scheme.IsSameAs(kxfS("ws"), StringActionFlag::IgnoreCase) || scheme.IsSameAs(kxfS("wss"), StringActionFlag::IgnoreCase))
		{
			// Same as CURLWebRequest::SetConnectOnly(WebRequestConnectOnly::EnabledWithResponse)
			m_Handle.SetOption(CURLOPT_CONNECT_ONLY, 2);

			// WebSockets usually send a whole pack of independent data so overwrite mode by default is better
			if (m_ReceiveMode == WebRequestReceiveMode::Default)
			{
				m_ReceiveMode = WebRequestReceiveMode::Overwrite;
			}
		}

		const int statusCode = ::curl_easy_perform(*m_Handle);
		UpdateStatusText(statusCode);

		// Get server response code
		const auto responseStatus = m_Response->GetResponseCode();
		const auto effectiveProtocol = m_Response->GetProtocol();

		// Decide what to do next
		if (effectiveProtocol == WebRequestProtocol::HTTP || effectiveProtocol == WebRequestProtocol::HTTPS)
		{
			HTTPStatus httpStatus = responseStatus ? static_cast<HTTPStatusCode>(*responseStatus) : HTTPStatusCode::Unknown;
			if (httpStatus == HTTPStatusCode::Unauthorized || httpStatus == HTTPStatusCode::ProxyAuthenticationRequired)
			{
				m_AuthChallenge.emplace(*this, httpStatus == HTTPStatusCode::ProxyAuthenticationRequired ? WebAuthChallengeSource::ProxyServer : WebAuthChallengeSource::TargetServer);
				ChangeStateAndNotify(WebRequestState::Unauthorized);

				return;
			}
		}

		switch (statusCode)
		{
			case CURLE_OK:
			{
				WebRequestState state = WebRequestState::None;
				if (effectiveProtocol == WebRequestProtocol::WS || effectiveProtocol == WebRequestProtocol::WSS)
				{
					state = HandleWebSocket();
				}
				else
				{
					state = WebRequestState::Completed;
				}
				ChangeStateAndNotify(state);

				break;
			}
			case CURLE_ABORTED_BY_CALLBACK:
			{
				ChangeStateAndNotify(WebRequestState::Cancelled);
				break;
			}
			case CURLE_LOGIN_DENIED:
			{
				// HTTP(S) unauthorized case is handled above, this case is for other protocols.
				m_AuthChallenge.emplace(*this, WebAuthChallengeSource::None);
				ChangeStateAndNotify(WebRequestState::Unauthorized);

				break;
			}
			default:
			{
				ChangeStateAndNotify(WebRequestState::Failed);
				break;
			}
		};
	}
	void CURLWebRequest::DoResetState()
	{
		m_AuthChallenge.reset();
		m_Response.emplace(*this);
		m_ResponseHeaders.clear();
		m_StatusTextBuffer.fill(0);
		m_StatusCode.reset();

		m_BytesReceived = -1;
		m_BytesExpectedToReceive = -1;
		m_BytesSent = -1;
		m_BytesExpectedToSend = -1;

		// WebSocket
		m_WebSocketsState = WebRequestState::None;
	}

	CURLWebRequest::CURLWebRequest(CURLWebSession& session, const std::vector<WebRequestHeader>& commonHeaders, const URI& uri)
		:m_Session(session), m_Handle(CURL::Private::HandleType::Easy)
	{
		static_assert(std::is_same_v<TCURLOffset, curl_off_t>, "'TCURLOffset' and 'curl_off_t' are not the same type");

		if (m_Handle)
		{
			// Make the session object handle events if this object doesn't
			m_EvtHandler.SetNextHandler(&m_Session);

			// Copy common headers
			m_RequestHeaders = commonHeaders;

			// Upload callback
			m_Handle.SetOption(CURLOPT_READDATA, this);
			m_Handle.SetOption(CURLOPT_READFUNCTION, &CURLWebRequest::OnReadDataCB);

			// Download callback
			m_Handle.SetOption(CURLOPT_WRITEDATA, this);
			m_Handle.SetOption(CURLOPT_WRITEFUNCTION, &CURLWebRequest::OnWriteDataCB);

			// Response headers callback
			m_Handle.SetOption(CURLOPT_HEADERDATA, this);
			m_Handle.SetOption(CURLOPT_HEADERFUNCTION, &CURLWebRequest::OnReceiveHeaderCB);

			// Progress function
			m_Handle.SetOption(CURLOPT_XFERINFODATA, this);
			m_Handle.SetOption(CURLOPT_XFERINFOFUNCTION, &CURLWebRequest::OnProgressNotifyCB);
			m_Handle.SetOption(CURLOPT_NOPROGRESS, false);

			// Set status text buffer
			static_assert(Utility::ArraySize<decltype(m_StatusTextBuffer)>::value >= CURL_ERROR_SIZE, "m_StatusTextBuffer must be at CURL_ERROR_SIZE");

			m_StatusTextBuffer.fill(0);
			m_Handle.SetOption(CURLOPT_ERRORBUFFER, m_StatusTextBuffer.data());

			// Set default parameters
			CURLWebRequest::SetURI(uri);
			CURLWebRequest::SetReceiveStorage(WebRequestStorage::Memory);

			m_Handle.SetOption(CURLOPT_PRIVATE, this);
			m_Handle.SetOption(CURLOPT_ACCEPT_ENCODING, "");

			// Mark as idle
			m_State = WebRequestState::Idle;
		}
	}
	CURLWebRequest::~CURLWebRequest() noexcept
	{
		// To make sure any active WebSocket thread gets released
		m_WebSocketsState = WebRequestState::Cancelled;

		DoFreeRequestHeaders();
	}

	// IWebRequest: Common
	std::shared_ptr<IAsyncTask> CURLWebRequest::Start()
	{
		if (m_State == WebRequestState::Idle)
		{
			DoResetState();
			ChangeStateAndNotify(WebRequestState::Started);

			DoSetRequestHeaders();
			return m_Session.StartRequest(*this);
		}
		return nullptr;
	}
	bool CURLWebRequest::Pause()
	{
		if (m_State == WebRequestState::Active)
		{
			m_NextState = WebRequestState::Paused;
		}
		return false;
	}
	bool CURLWebRequest::Resume()
	{
		if (m_State == WebRequestState::Paused)
		{
			m_NextState = WebRequestState::Resumed;
			return true;
		}
		return false;
	}
	bool CURLWebRequest::Cancel()
	{
		if (m_State == WebRequestState::Active || m_State == WebRequestState::Paused)
		{
			m_NextState = WebRequestState::Cancelled;
			return true;
		}
		return false;
	}

	// IWebRequest: Request options
	bool CURLWebRequest::SetHeader(const WebRequestHeader& header, FlagSet<WebRequestHeaderFlag> flags)
	{
		return CURLWebSession::SetHeader(m_RequestHeaders, header, flags);
	}
	void CURLWebRequest::ClearHeaders()
	{
		m_RequestHeaders.clear();
	}

	bool CURLWebRequest::SetSendStorage(WebRequestStorage storage)
	{
		if (m_State == WebRequestState::Idle)
		{
			switch (storage)
			{
				case WebRequestStorage::Memory:
				case WebRequestStorage::Stream:
				case WebRequestStorage::FileSystem:
				{
					m_SendStorage = storage;
					return true;
				}
			};
		}
		return false;
	}
	bool CURLWebRequest::SetSendSource(std::shared_ptr<IInputStream> stream)
	{
		if (m_State == WebRequestState::Idle)
		{
			if (m_SendStream = std::move(stream))
			{
				m_SendData = {};
				m_SendStorage = WebRequestStorage::Stream;

				return true;
			}
		}
		return false;
	}
	bool CURLWebRequest::SetSendSource(const String& data)
	{
		if (m_State == WebRequestState::Idle)
		{
			auto encodedData = m_Handle.EscapeString(data.ToUTF8());
			if (!encodedData.empty() || data.IsEmpty())
			{
				if (CURLWebRequest::SetSendSource(std::make_shared<MemoryInputStream>(encodedData.data(), encodedData.size())))
				{
					m_SendData = std::move(encodedData);
					m_SendStorage = WebRequestStorage::Memory;

					return true;
				}
			}
		}
		return false;
	}
	bool CURLWebRequest::SetSendSource(const FSPath& filePath)
	{
		if (m_State == WebRequestState::Idle && filePath)
		{
			if (auto fs = m_Session.GetFileSystem())
			{
				if (CURLWebRequest::SetSendSource(fs->OpenToRead(filePath)))
				{
					m_SendData = {};
					m_SendStorage = WebRequestStorage::FileSystem;

					return true;
				}
			}
		}
		return false;
	}

	bool CURLWebRequest::SetReceiveStorage(WebRequestStorage storage, WebRequestReceiveMode receiveMode)
	{
		if (m_State == WebRequestState::Idle)
		{
			switch (storage)
			{
				case WebRequestStorage::Memory:
				case WebRequestStorage::Stream:
				case WebRequestStorage::FileSystem:
				{
					m_ReceiveStorage = storage;
					m_ReceiveMode = receiveMode;
					return true;
				}
			};
		}
		return false;
	}
	bool CURLWebRequest::SetReceiveTarget(std::shared_ptr<IOutputStream> stream, WebRequestReceiveMode receiveMode)
	{
		if (m_State == WebRequestState::Idle)
		{
			if (stream)
			{
				m_ReceiveStream = std::move(stream);
				m_ReceiveStorage = WebRequestStorage::Stream;
				m_ReceiveMode = receiveMode;

				return true;
			}
		}
		return false;
	}
	bool CURLWebRequest::SetReceiveTarget(const FSPath& filePath, WebRequestReceiveMode receiveMode)
	{
		if (m_State == WebRequestState::Idle && filePath)
		{
			if (auto fs = m_Session.GetFileSystem())
			{
				if (m_ReceiveStream = fs->OpenToWrite(filePath))
				{
					m_ReceiveStorage = WebRequestStorage::FileSystem;
					m_ReceiveMode = receiveMode;
				}
			}
		}
		return false;
	}

	// IWebRequest: Progress
	TransferRate CURLWebRequest::GetSendRate() const
	{
		if (auto rate = m_Handle.GetOptionInt64(CURLINFO_SPEED_UPLOAD_T))
		{
			return TransferRate::FromBytes(*rate);
		}
		return {};
	}
	TransferRate CURLWebRequest::GetReceiveRate() const
	{
		if (auto rate = m_Handle.GetOptionInt64(CURLINFO_SPEED_DOWNLOAD_T))
		{
			return TransferRate::FromBytes(*rate);
		}
		return {};
	}

	// IWebRequestWebSocket
	void CURLWebRequest::CloseWebSocket()
	{
		size_t sent = 0;
		if (::curl_ws_send(*m_Handle, nullptr, 0, &sent, 0, CURLWS_CLOSE) != CURLE_OK)
		{
			// If the WebSocket didn't close itself, this will stop its thread anyway effectively abandoning the socket
			m_WebSocketsState = WebRequestState::Failed;
		}
	}
	bool CURLWebRequest::WebSocketSendText(const String& text)
	{
		auto utf8 = text.ToUTF8();

		size_t sent = 0;
		return ::curl_ws_send(*m_Handle, utf8.data(), utf8.size(), &sent, 0, CURLWS_TEXT) == CURLE_OK;
	}
	bool CURLWebRequest::WebSocketSendData(const std::span<std::byte> buffer)
	{
		size_t sent = 0;
		return ::curl_ws_send(*m_Handle, buffer.data(), buffer.size_bytes(), &sent, 0, CURLWS_BINARY) == CURLE_OK;
	}

	// IWebRequestOptions
	bool CURLWebRequest::SetURI(const URI& uri)
	{
		if (m_Session.m_BaseURI && uri.IsReference())
		{
			// Resolve with base URI
			auto resolvedURI = m_Session.ResolveURI(uri);
			if (m_Handle.SetOption(CURLOPT_URL, resolvedURI.BuildURI()))
			{
				m_URI = std::move(resolvedURI);
				return true;
			}
		}
		else
		{
			// No base URI specified or the provided for this request URI is an absolute address, use the URI as is
			if (m_Handle.SetOption(CURLOPT_URL, uri.BuildURI()))
			{
				m_URI = uri;
				return true;
			}
		}
		return false;
	}
	bool CURLWebRequest::SetPort(uint16_t port)
	{
		return m_Handle.SetOption(CURLOPT_PORT, port);
	}
	bool CURLWebRequest::SetMethod(const String& method)
	{
		m_Method = method;
		return true;
	}
	bool CURLWebRequest::SetDefaultProtocol(const String& protocol)
	{
		return m_Handle.SetOption(CURLOPT_DEFAULT_PROTOCOL, protocol);
	}
	bool CURLWebRequest::SetAllowedProtocols(FlagSet<WebRequestProtocol> protocols)
	{
		return m_Handle.SetOption(CURLOPT_PROTOCOLS_STR, CURL::Private::MapProtocolSet(protocols));
	}
	bool CURLWebRequest::SetHTTPVersion(WebRequestHTTPVersion option)
	{
		switch (option)
		{
			case WebRequestHTTPVersion::Any:
			{
				return m_Handle.SetOption(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);
			}
			case WebRequestHTTPVersion::Version1_0:
			{
				return m_Handle.SetOption(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
			}
			case WebRequestHTTPVersion::Version1_1:
			{
				return m_Handle.SetOption(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
			}
			case WebRequestHTTPVersion::Version2:
			{
				return m_Handle.SetOption(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2);
			}
			case WebRequestHTTPVersion::Version2TLS:
			{
				return m_Handle.SetOption(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2TLS);
			}
			case WebRequestHTTPVersion::Version3:
			{
				return m_Handle.SetOption(CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_3);
			}
		};
		return false;
	}
	bool CURLWebRequest::SetIPVersion(WebRequestIPVersion option)
	{
		switch (option)
		{
			case WebRequestIPVersion::Any:
			{
				return m_Handle.SetOption(CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);
			}
			case WebRequestIPVersion::IPv4:
			{
				return m_Handle.SetOption(CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
			}
			case WebRequestIPVersion::IPv6:
			{
				return m_Handle.SetOption(CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
			}
		};
		return false;
	}
	bool CURLWebRequest::SetConnectOnly(WebRequestConnectOnly option)
	{
		switch (option)
		{
			case WebRequestConnectOnly::Disabled:
			{
				return m_Handle.SetOption(CURLOPT_CONNECT_ONLY, 0);
			}
			case WebRequestConnectOnly::Enabled:
			{
				return m_Handle.SetOption(CURLOPT_CONNECT_ONLY, 1);
			}
			case WebRequestConnectOnly::EnabledWithResponse:
			{
				return m_Handle.SetOption(CURLOPT_CONNECT_ONLY, 2);
			}
		};
		return false;
	}

	bool CURLWebRequest::SetServiceName(const String& name)
	{
		return m_Handle.SetOption(CURLOPT_SERVICE_NAME, name);
	}
	bool CURLWebRequest::SetAllowRedirection(WebRequestOption2 option)
	{
		m_FollowLocation = option;
		return m_Handle.SetOption(CURLOPT_FOLLOWLOCATION, option == WebRequestOption2::Enabled);
	}
	bool CURLWebRequest::SetRedirectionProtocols(FlagSet<WebRequestProtocol> protocols)
	{
		return m_Handle.SetOption(CURLOPT_REDIR_PROTOCOLS_STR, CURL::Private::MapProtocolSet(protocols));
	}
	bool CURLWebRequest::SetResumeOffset(DataSize offset)
	{
		return offset.IsValid() && m_Handle.SetOption(CURLOPT_RESUME_FROM_LARGE, offset.ToBytes());
	}

	bool CURLWebRequest::SetRequestTimeout(const TimeSpan& timeout)
	{
		return m_Handle.SetOption(CURLOPT_TIMEOUT_MS, timeout.IsPositive() ? timeout.GetMilliseconds() : 0);
	}
	bool CURLWebRequest::SetConnectionTimeout(const TimeSpan& timeout)
	{
		return m_Handle.SetOption(CURLOPT_CONNECTTIMEOUT_MS, timeout.IsPositive() ? timeout.GetMilliseconds() : 0);
	}

	bool CURLWebRequest::SetMaxSendRate(const TransferRate& rate)
	{
		return m_Handle.SetOption(CURLOPT_MAX_SEND_SPEED_LARGE, rate.IsValid() ? rate.ToBytes() : 0);
	}
	bool CURLWebRequest::SetMaxReceiveRate(const TransferRate& rate)
	{
		return m_Handle.SetOption(CURLOPT_MAX_RECV_SPEED_LARGE, rate.IsValid() ? rate.ToBytes() : 0);
	}

	bool CURLWebRequest::SetKeepAlive(WebRequestOption2 option)
	{
		return m_Handle.SetOption(CURLOPT_TCP_KEEPALIVE, option == WebRequestOption2::Enabled);
	}
	bool CURLWebRequest::SetKeepAliveIdle(const TimeSpan& interval)
	{
		return m_Handle.SetOption(CURLOPT_TCP_KEEPIDLE, interval.IsPositive() ? interval.GetSeconds() : 60);
	}
	bool CURLWebRequest::SetKeepAliveInterval(const TimeSpan& interval)
	{
		return m_Handle.SetOption(CURLOPT_TCP_KEEPINTVL, interval.IsPositive() ? interval.GetSeconds() : 60);
	}

	// IWebRequestAuthOptions
	bool CURLWebRequest::SetAuthMethod(WebRequestAuthMethod method)
	{
		switch (method)
		{
			case WebRequestAuthMethod::Basic:
			{
				return m_Handle.SetOption(CURLOPT_HTTPAUTH, CURLAUTH_ONLY|CURLAUTH_BASIC);
			}
			case WebRequestAuthMethod::NTLM:
			{
				return m_Handle.SetOption(CURLOPT_HTTPAUTH, CURLAUTH_ONLY|CURLAUTH_NTLM);
			}
			case WebRequestAuthMethod::Digest:
			{
				return m_Handle.SetOption(CURLOPT_HTTPAUTH, CURLAUTH_ONLY|CURLAUTH_DIGEST);
			}
			case WebRequestAuthMethod::Bearer:
			{
				return m_Handle.SetOption(CURLOPT_HTTPAUTH, CURLAUTH_ONLY|CURLAUTH_BEARER);
			}
			case WebRequestAuthMethod::Negotiate:
			{
				return m_Handle.SetOption(CURLOPT_HTTPAUTH, CURLAUTH_ONLY|CURLAUTH_NEGOTIATE);
			}
		};
		return false;
	}
	bool CURLWebRequest::SetAuthMethods(FlagSet<WebRequestAuthMethod> methods)
	{
		FlagSet<uint32_t> curlFlags;
		curlFlags.Add(CURLAUTH_BASIC, methods & WebRequestAuthMethod::Basic);
		curlFlags.Add(CURLAUTH_NTLM, methods & WebRequestAuthMethod::NTLM);
		curlFlags.Add(CURLAUTH_DIGEST, methods & WebRequestAuthMethod::Digest);
		curlFlags.Add(CURLAUTH_BEARER, methods & WebRequestAuthMethod::Bearer);
		curlFlags.Add(CURLAUTH_NEGOTIATE, methods & WebRequestAuthMethod::Negotiate);

		return m_Handle.SetOption(CURLOPT_HTTPAUTH, *curlFlags);
	}

	bool CURLWebRequest::SetUserName(const String& userName)
	{
		return m_Handle.SetOption(CURLOPT_USERNAME, userName);
	}
	bool CURLWebRequest::SetUserPassword(const String& userPassword)
	{
		return m_Handle.SetOption(CURLOPT_PASSWORD, userPassword);
	}

	// IWebRequestSecurityOptions
	bool CURLWebRequest::SetUseSSL(WebRequestOption3 option)
	{
		switch (option)
		{
			case WebRequestOption3::Diabled:
			{
				return m_Handle.SetOption(CURLOPT_USE_SSL, CURLUSESSL_NONE);
			}
			case WebRequestOption3::Enabled:
			{
				return m_Handle.SetOption(CURLOPT_USE_SSL, CURLUSESSL_TRY);
			}
			case WebRequestOption3::Required:
			{
				return m_Handle.SetOption(CURLOPT_USE_SSL, CURLUSESSL_ALL);
			}
		};
		return false;
	}
	bool CURLWebRequest::SetSSLVersion(WebRequestSSLVersion version)
	{
		switch (version)
		{
			case WebRequestSSLVersion::Default:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
			}
			case WebRequestSSLVersion::SSLv2:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv2);
			}
			case WebRequestSSLVersion::SSLv3:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv3);
			}
			case WebRequestSSLVersion::TLSv1_x:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
			}
			case WebRequestSSLVersion::TLSv1_0:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_0);
			}
			case WebRequestSSLVersion::TLSv1_1:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_1);
			}
			case WebRequestSSLVersion::TLSv1_2:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
			}
			case WebRequestSSLVersion::TLSv1_3:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);
			}
		};
		return false;
	}
	bool CURLWebRequest::SetMaxSSLVersion(WebRequestSSLVersion version)
	{
		switch (version)
		{
			case WebRequestSSLVersion::Default:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_DEFAULT);
			}
			case WebRequestSSLVersion::TLSv1_0:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_0);
			}
			case WebRequestSSLVersion::TLSv1_1:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_1);
			}
			case WebRequestSSLVersion::TLSv1_2:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_2);
			}
			case WebRequestSSLVersion::TLSv1_3:
			{
				return m_Handle.SetOption(CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_3);
			}
		};
		return false;
	}

	bool CURLWebRequest::SetVerifyPeer(WebRequestOption2 option)
	{
		return m_Handle.SetOption(CURLOPT_SSL_VERIFYPEER, option == WebRequestOption2::Enabled);
	}
	bool CURLWebRequest::SetVerifyHost(WebRequestOption2 option)
	{
		return m_Handle.SetOption(CURLOPT_SSL_VERIFYHOST, option == WebRequestOption2::Enabled ? 2 : 0);
	}
	bool CURLWebRequest::SetVerifyStatus(WebRequestOption2 option)
	{
		return m_Handle.SetOption(CURLOPT_SSL_VERIFYSTATUS, option == WebRequestOption2::Enabled);
	}

	// IWebRequestWebSocketsOptions
	bool CURLWebRequest::SetRawMode(WebRequestOption2 option)
	{
		m_WebSocketsOptions.Mod(CURLWS_RAW_MODE, option == WebRequestOption2::Enabled);
		return m_Handle.SetOption(CURLOPT_WS_OPTIONS, *m_WebSocketsOptions);
	}
	bool CURLWebRequest::SetAutoPong(WebRequestOption2 option)
	{
		m_WebSocketsOptions.Mod(2, option == WebRequestOption2::Disabled); // TODO: Replace with CURLWS_NOAUTOPONG when it's going to be defined
		return m_Handle.SetOption(CURLOPT_WS_OPTIONS, *m_WebSocketsOptions);
	}
	bool CURLWebRequest::SetWSTimeout(TimeSpan timeout)
	{
		m_WebSocketsTimeOut = timeout;
		return true;
	}
}
