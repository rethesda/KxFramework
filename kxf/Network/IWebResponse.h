#pragma once
#include "Common.h"
#include "URI.h"
#include "HTTPStatus.h"
#include "kxf/RTTI/RTTI.h"
#include "kxf/General/BinarySize.h"

namespace kxf
{
	class IInputStream;
}

namespace kxf
{
	class IWebResponse: public RTTI::Interface<IWebResponse>
	{
		KxRTTI_DeclareIID(IWebResponse, {0x24f29008, 0x76c7, 0x42e7, {0x9f, 0x6b, 0x3, 0xcc, 0x4, 0xf3, 0xda, 0x33}});

		public:
			virtual bool IsNull() const = 0;

			virtual URI GetURI() const = 0;
			virtual String GetSuggestedFileName() const = 0;
			virtual String GetHeader(const String& name) const = 0;
			virtual HTTPStatus GetStatus() const = 0;

			virtual String GetMIMEType() const = 0;
			virtual BinarySize GetContentLength() const = 0;

			virtual IInputStream& GetStream() = 0;

		public:
			explicit operator bool() const
			{
				return !IsNull();
			}
			bool operator!() const
			{
				return IsNull();
			}
	};
}

namespace kxf
{
	class NullWebResponse final: public IWebResponse
	{
		public:
			static IWebResponse& Get();

		public:
			bool IsNull() const override
			{
				return true;
			}

			URI GetURI() const override
			{
				return {};
			}
			String GetSuggestedFileName() const override
			{
				return {};
			}
			String GetHeader(const String& name) const override
			{
				return {};
			}
			HTTPStatus GetStatus() const override
			{
				return {};
			}

			String GetMIMEType() const override
			{
				return {};
			}
			BinarySize GetContentLength() const override
			{
				return {};
			}

			IInputStream& GetStream() override;
	};
}