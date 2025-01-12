#pragma once
#include "Common.h"
#include "HashValue.h"
#include "SecretValue.h"
#include "kxf/Core/DataSize.h"
#include "kxf/RTTI/RTTI.h"
#include "kxf/IO/IStream.h"

namespace kxf::Crypto
{
	class KXF_API_CRYPTO IHashCalculator: public RTTI::Interface<IHashCalculator>
	{
		kxf_RTTI_DeclareIID(IHashCalculator, {0x4abebdc6, 0x7550, 0x4c61, {0xbf, 0xd7, 0xac, 0xbc, 0xec, 0x1c, 0x44, 0xd7}});

		public:
			template<size_t bitLength>
			static HashValue<bitLength> ComputeDefault(IHashCalculator& calc, IInputStream& stream, uint64_t seed, const SecretValue& secret)
			{
				if (calc.Initialize(seed, secret))
				{
					std::array<std::byte, DataSize::FromKB(64).ToBytes()> buffer;
					while (stream.CanRead())
					{
						DataSize lastRead = stream.Read(buffer.data(), buffer.size()).LastRead();
						if (lastRead != 0)
						{
							if (!calc.Update({buffer.data(), buffer.data() + lastRead.ToBytes()}))
							{
								return {};
							}
						}
						else
						{
							break;
						}
					}

					HashValue<bitLength> hash;
					if (calc.Finalize(hash.as_span()))
					{
						return hash;
					}
				}
				return {};
			}

		public:
			virtual bool Initialize(uint64_t seed, const SecretValue& secret) = 0;
			virtual bool Update(std::span<const std::byte> input) = 0;
			virtual bool Finalize(std::span<std::byte> hash) = 0;

		public:
			template<size_t bitLength>
			auto Compute(IInputStream& stream, uint64_t seed, const SecretValue& secret)
			{
				return ComputeDefault<bitLength>(*this, stream, seed, secret);
			}
	};
}
