#include "kxf-pch.h"
#include "ScopedLoggerTarget.h"
#include "kxf/IO/StreamReaderWriter.h"
#include "kxf/FileSystem/NativeFileSystem.h"
#include "kxf/Threading/LockGuard.h"
#include <iostream>

namespace kxf::Private
{
	bool ScopedLoggerFlushControl::ShouldFlush(LogLevel logLevel) const noexcept
	{
		switch (logLevel)
		{
			case LogLevel::FlowControl:
			case LogLevel::Critical:
			case LogLevel::Warning:
			case LogLevel::Error:
			{
				return true;
			}
		};

		if (m_FlushThreshold != std::numeric_limits<size_t>::max() && m_WriteCount >= m_FlushThreshold)
		{
			return true;
		}
		return false;
	}
}

namespace kxf
{
	// IScopedLoggerTarget
	void ScopedLoggerConsoleTarget::Write(LogLevel logLevel, StringView str)
	{
		std::wcout << str << std::endl;
	}
	void ScopedLoggerConsoleTarget::Flush()
	{
		std::wcout.flush();
	}
}

namespace kxf
{
	String ScopedLoggerFileTarget::FormatTimestamp(DateTime timestamp, const TimeZoneOffset& tzOffset)
	{
		return timestamp.Format("%Y-%m-%d %H-%M-%S.%l", tzOffset);
	}

	ScopedLoggerFileTarget::ScopedLoggerFileTarget(std::shared_ptr<IOutputStream> stream)
		:m_Stream(std::move(stream))
	{
	}
	ScopedLoggerFileTarget::ScopedLoggerFileTarget(ScopedLoggerTLS& tls, IFileSystem& fs, const FSPath& directory)
	{
		auto timestamp = tls.GetTimestamp();
		auto directoryTimestamp = tls.GetGlobalContext().GetUnknownThreadContext().GetTimestamp();
		auto tzOffset = tls.GetGlobalContext().GetTimeOffset();

		String path;
		path.Format("[{}] {}-{}\\", FormatTimestamp(directoryTimestamp, tzOffset), tls.GetProcess().GetExecutablePath().GetName(), tls.GetProcess().GetID());
		if (!tls.IsUnknown())
		{
			path.Format("[{}] {}.log", FormatTimestamp(timestamp, tzOffset), tls.GetThread().GetID());
		}
		else
		{
			path.Format("[{}] UnknownContext.log", FormatTimestamp(timestamp, tzOffset));
		}

		m_Stream = fs.OpenToWrite(directory / path, IOStreamDisposition::CreateAlways, IOStreamShare::Read, FSActionFlag::CreateDirectoryTree|FSActionFlag::Recursive);
	}

	// IScopedLoggerTarget
	void ScopedLoggerFileTarget::Write(LogLevel logLevel, StringView str)
	{
		IO::OutputStreamWriter writer(*m_Stream);
		writer.WriteStringUTF8(String(str).Append('\n', 1));

		m_FlushControl.OnWrite();
		if (m_FlushControl.ShouldFlush(logLevel))
		{
			m_Stream->Flush();
			m_FlushControl.OnFlush();
		}
	}
	void ScopedLoggerFileTarget::Flush()
	{
		m_Stream->Flush();
		m_FlushControl.OnFlush();
	}
}

namespace kxf
{
	ScopedLoggerThreadedFileTarget::ScopedLoggerThreadedFileTarget(ScopedLoggerTLS& tls, IFileSystem& fs, const FSPath& directory)
		:ScopedLoggerFileTarget(nullptr)
	{
		String path;
		if (tls.IsUnknown())
		{
			path.Format("UnknownContext.log");
		}
		else
		{
			path.Format("{}.log", tls.GetThread().GetID());
		}

		m_Stream = fs.OpenToWrite(directory / path, IOStreamDisposition::CreateAlways, IOStreamShare::Read);
	}
}

namespace kxf
{
	// IScopedLoggerTarget
	void ScopedLoggerSingleFileTarget::Write(LogLevel logLevel, StringView str)
	{
		WriteLockGuard lock(m_Lock);

		IO::OutputStreamWriter writer(*m_Stream);
		writer.WriteStringUTF8(String(str).Append('\n', 1));

		m_FlushControl.OnWrite();
		if (m_FlushControl.ShouldFlush(logLevel))
		{
			m_Stream->Flush();
			m_FlushControl.OnFlush();
		}
	}
	void ScopedLoggerSingleFileTarget::Flush()
	{
		WriteLockGuard lock(m_Lock);

		m_Stream->Flush();
		m_FlushControl.OnFlush();
	}
}
