#include "kxf-pch.h"
#include "ScopedLoggerContext.h"
#include "ScopedLoggerTarget.h"

namespace kxf
{
	// IScopedLoggerContext
	std::shared_ptr<IScopedLoggerTarget> ScopedLoggerFileContext::CreateLogTarget(ScopedLoggerTLS& tls)
	{
		return std::make_shared<ScopedLoggerFileTarget>(tls, *m_FileSystem, m_LogDirectory);
	}
}

namespace kxf
{
	// ScopedLoggerSingleFileContext
	ScopedLoggerSingleFileContext::ScopedLoggerSingleFileContext(std::shared_ptr<IOutputStream> stream)
	{
		m_Target = std::make_shared<ScopedLoggerSingleFileTarget>(std::move(stream));
	}
	ScopedLoggerSingleFileContext::ScopedLoggerSingleFileContext(IFileSystem& fs, const FSPath& filePath)
	{
		auto stream = fs.OpenToWrite(filePath, IOStreamDisposition::CreateAlways, IOStreamShare::Read, FSActionFlag::CreateDirectoryTree|FSActionFlag::Recursive);
		m_Target = std::make_shared<ScopedLoggerSingleFileTarget>(std::move(stream));
	}
}

namespace kxf
{
	// IScopedLoggerContext
	std::shared_ptr<IScopedLoggerTarget> ScopedLoggerAggregateContext::CreateLogTarget(ScopedLoggerTLS& tls)
	{
		return m_AggregateTarget;
	}
}

namespace kxf
{
	// IScopedLoggerContext
	std::shared_ptr<IScopedLoggerTarget> ScopedLoggerThreadedContext::CreateLogTarget(ScopedLoggerTLS& tls)
	{
		if (!m_Directory)
		{
			auto& globalContext = tls.GetGlobalContext();
			auto directoryTimestamp = globalContext.GetUnknownThreadContext().GetTimestamp();
			auto tzOffset = globalContext.GetTimeOffset();

			m_Directory = Format("{} {}-{} [{}]\\", m_BaseName,
								 tls.GetProcess().GetExecutablePath().GetName(),
								 tls.GetProcess().GetID(),
								 ScopedLoggerFileTarget::FormatTimestamp(directoryTimestamp, tzOffset)
			);
			m_FileSystem->CreateDirectory(m_Directory, FSActionFlag::Recursive);
		}

		return std::make_shared<ScopedLoggerThreadedFileTarget>(tls, *m_FileSystem, m_Directory);
	}
}
