#pragma once
#include "kxf/Common.hpp"

namespace kxf
{
	enum class EventCategory: uint32_t
	{
		None = 0,

		UserInterface = FlagSetValue<EventCategory>(0),
		UserInput = FlagSetValue<EventCategory>(1),
		Socket = FlagSetValue<EventCategory>(2),
		Timer = FlagSetValue<EventCategory>(3),
		Thread = FlagSetValue<EventCategory>(4),
		Unknown = FlagSetValue<EventCategory>(5),
		Clipboard = FlagSetValue<EventCategory>(6),

		NativeEvents = UserInterface|UserInput,
		Everything = UserInterface|Timer|Thread|Socket|Unknown|Clipboard|UserInput
	};
	kxf_FlagSet_Declare(EventCategory);

	enum class BindEventFlag: uint32_t
	{
		None = 0,

		Direct = 1,
		Queued = 6,
		Auto = 12,

		Unique = 1 << 16,
		Blocking = 1 << 17,
		AlwaysSkip = 1 << 18,
		OneShot = 1 << 19
	};
	kxf_FlagSet_Declare(BindEventFlag);

	enum class BindSignalFlag: uint32_t
	{
		None = 0,

		EventContext = 1 << 0
	};
	kxf_FlagSet_Declare(BindSignalFlag);

	enum class ProcessEventFlag: uint32_t
	{
		None = 0,

		// Try to process the event in this handler and all those chained to it
		Locally = 1 << 0,

		// Processes an event and handles any exceptions that occur in the process
		HandleExceptions = 1 << 1
	};
	kxf_FlagSet_Declare(ProcessEventFlag);

	enum class SignalParametersSemantics
	{
		Copy,
		Move
	};
}
