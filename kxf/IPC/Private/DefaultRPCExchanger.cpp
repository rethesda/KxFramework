#include "KxfPCH.h"
#include "DefaultRPCExchanger.h"
#include "DefaultRPCExchangerWindow.h"
#include "../DefaultRPCEvent.h"
#include "../SharedMemory.h"
#include "kxf/IO/IStream.h"
#include "kxf/IO/NullStream.h"
#include "kxf/System/Win32Error.h"
#include "kxf/EventSystem/IEvtHandler.h"
#include "kxf/Application/ICoreApplication.h"
#include <Windows.h>

namespace
{
	constexpr kxf::XChar g_SharedPrefix[] = wxS("kxf::DefaultRPCExchanger");
}

namespace kxf
{
	size_t DefaultRPCExchanger::GetControlBufferSize() const
	{
		return 64;
	}
	String DefaultRPCExchanger::GetControlBufferName() const
	{
		return String::Format(wxS("%1:%2-ControlBuffer"), g_SharedPrefix, m_SessionID.ToString(UUIDFormat::CurlyBraces));
	}
	String DefaultRPCExchanger::GetResultBufferName() const
	{
		return String::Format(wxS("%1:%2-ResultBuffer"), g_SharedPrefix, m_SessionID.ToString(UUIDFormat::CurlyBraces));
	}
	String DefaultRPCExchanger::GetSessionMutexName() const
	{
		return String::Format(wxS("%1:%2-SessionMutex"), g_SharedPrefix, m_SessionID.ToString(UUIDFormat::CurlyBraces));
	}

	void DefaultRPCExchanger::OnInitialize(const UniversallyUniqueID& sessionID, IEvtHandler& evtHandler, KernelObjectNamespace ns)
	{
		m_SessionID = sessionID;
		m_EvtHandler = &evtHandler;
		m_KernelObjectNamespace = ns;
	}
	void DefaultRPCExchanger::OnTerminate()
	{
		if (m_ReceivingWindow)
		{
			m_ReceivingWindow->Destroy();
			m_ReceivingWindow = nullptr;
		}
		m_SessionMutex.Destroy();
		m_ControlBuffer.Free();
	}

	void DefaultRPCExchanger::OnDataRecievedCommon(IInputStream& stream, DefaultRPCEvent& event)
	{
		auto& procedure = event.m_Procedure;
		Serialization::ReadObject(stream, procedure);

		if (procedure)
		{
			// Set stream with serialized parameters for an event handler to read from
			if (procedure.HasParameters())
			{
				event.RawSetProcedureParameters(stream);
			}

			// Call event handler if any
			if (m_EvtHandler->ProcessEvent(event, procedure.GetProcedureID()) && procedure.HasResult())
			{
				// If we had processed the event get serialized result and write it into shared result buffer
				if (IInputStream& resultStream = event.RawGetProcedureResult())
				{
					const uint64_t size = resultStream.GetSize().ToBytes();
					m_ResultBuffer.Allocate(size + sizeof(size), MemoryProtection::RW, GetResultBufferName(), m_KernelObjectNamespace);

					// Write allocated size and the result
					auto stream = m_ResultBuffer.GetOutputStream();
					Serialization::WriteObject(*stream, size);
					stream->Write(resultStream);
				}
			}
		}
	}
	IInputStream& DefaultRPCExchanger::SendData(void* windowHandle, const DefaultRPCProcedure& procedure, const wxStreamBuffer& buffer)
	{
		COPYDATASTRUCT data = {};
		data.lpData = buffer.GetBufferStart();
		data.cbData = buffer.GetBufferSize();

		m_ResultStream.reset();
		m_ResultBuffer.ZeroBuffer();
		Win32Error::SetLastError(Win32Error::Success());
		::SendMessageW(reinterpret_cast<HWND>(windowHandle), WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&data));

		if (procedure.HasResult() && Win32Error::GetLastError().IsSuccess() && m_ResultBuffer.Open(GetResultBufferName(), 0, MemoryProtection::RW, m_KernelObjectNamespace))
		{
			// Retrieve the actual size of the result
			uint64_t actualSize = 0;

			// Assume at least 'sizeof(actualSize)' here since we don't know the size and we don't need to read more anyway
			MemoryInputStream stream(m_ResultBuffer.GetBuffer(), sizeof(actualSize));
			auto read = Serialization::ReadObject(stream, actualSize);

			if (actualSize != 0)
			{
				m_ResultStream.emplace(reinterpret_cast<const uint8_t*>(m_ResultBuffer.GetBuffer()) + read, actualSize);
				return *m_ResultStream;
			}
		}
		return NullInputStream::Get();
	}
}