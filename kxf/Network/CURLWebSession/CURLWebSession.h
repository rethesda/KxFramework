#pragma once
#include "Common.h"
#include "../Private/BasicWebSession.h"
#include "kxf/Core/ILibraryInfo.h"

namespace kxf
{
	class KXF_API_NETWORK CURLWebSession final: public RTTI::DynamicImplementation<CURLWebSession, Private::BasicWebSession, ILibraryInfo>
	{
		friend class CURLWebRequest;

		private:
			std::vector<WebRequestHeader> m_CommonHeaders;
			std::shared_ptr<IFileSystem> m_FileSystem;
			URI m_BaseURI;

		private:
			URI ResolveURI(const URI& uri) const
			{
				return URI(uri).Resolve(m_BaseURI);
			}

		public:
			CURLWebSession(std::shared_ptr<IAsyncTaskExecutor> taskExecutor = {}, const URI& uri = {});

		public:
			// IWebSession
			void SetBaseURI(const URI& uri) override
			{
				m_BaseURI = uri;
			}
			std::shared_ptr<IWebRequest> CreateRequest(const URI& uri) override;

			bool SetCommonHeader(const WebRequestHeader& header, FlagSet<WebRequestHeaderFlag> flags) override
			{
				return SetHeader(m_CommonHeaders, header, flags);
			}
			void ClearCommonHeaders() override
			{
				m_CommonHeaders.clear();
			}

			std::shared_ptr<IFileSystem> GetFileSystem() const override;
			void SetFileSystem(std::shared_ptr<IFileSystem> fileSystem) override;

			String GetDefaultUserAgent() const override;
			void* GetNativeHandle() const override
			{
				return nullptr;
			}

			// ILibraryInfo
			String GetName() const override;
			Version GetVersion() const override;
			URI GetHomePage() const override;
			uint32_t GetAPILevel() const override;

			String GetLicense() const override;
			String GetLicenseName() const override;
			String GetCopyright() const override;
	};
}
