#pragma once
#include "Common.h"
#include "IRPCEvent.h"
#include "Private/RPCExchange.h"

namespace kxf
{
	class IRPCServer;
	class IEvtHandler;
}

namespace kxf
{
	class IRPCClient: public RTTI::Interface<IRPCClient>
	{
		KxRTTI_DeclareIID(IRPCClient, {0x57b31a53, 0x4fbd, 0x454f, {0x93, 0xb, 0xd0, 0x5b, 0xfb, 0x74, 0xe, 0x57}});

		public:
			virtual bool IsConnectedToServer() const = 0;
			virtual bool ConnectToServer(const UniversallyUniqueID& sessionID, IEvtHandler& evtHandler, KernelObjectNamespace ns = KernelObjectNamespace::Local) = 0;
			virtual void DisconnectFromServer() = 0;

			virtual IInputStream& RawInvokeProcedure(const EventID& procedureID, IInputStream& parameters, size_t parametersCount, bool hasResult) = 0;

		public:
			template<class TReturn = void, class... Args>
			TReturn InvokeProcedure(const EventID& procedureID, Args&&... arg)
			{
				return IPC::Private::InvokeProcedure<TReturn>([&](MemoryOutputStream& parametersStream, size_t parametersCount, bool hasResult) -> IInputStream&
				{
					MemoryInputStream parametersInputStream(parametersStream);
					return RawInvokeProcedure(procedureID, parametersInputStream, parametersCount, hasResult);
				}, procedureID, std::forward<Args>(arg)...);
			}
	};
}