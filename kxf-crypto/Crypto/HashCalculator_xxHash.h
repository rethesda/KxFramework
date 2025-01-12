#pragma once
#include "Common.h"
#include "kxf/IO/IStream.h"
#include "kxf/Crypto/HashValue.h"
#include "kxf/Crypto/IHashCalculator.h"

#define XXH_STATIC_LINKING_ONLY
#include <xxhash.h>

namespace kxf::Crypto
{
	template
	<
		class THashType,
		class TState,
		auto createState,
		auto freeState,
		auto resetState,
		auto updateState,
		auto finalizeState
	>
	class HashCalculator_xxHash1 final: public IHashCalculator
	{
		private:
			TState* m_State = nullptr;

		public:
			HashCalculator_xxHash1() noexcept
			{
				m_State = std::invoke(createState);
			}
			~HashCalculator_xxHash1()
			{
				if (m_State)
				{
					std::invoke(freeState, m_State);
				}
			}

		public:
			// IHashCalculator
			bool Initialize(uint64_t seed, const SecretValue& secret) noexcept override
			{
				if (m_State)
				{
					return std::invoke(resetState, m_State, static_cast<THashType>(seed)) == XXH_errorcode::XXH_OK;
				}
				return false;
			}
			bool Update(std::span<const std::byte> input) noexcept override
			{
				return std::invoke(updateState, m_State, input.data(), input.size_bytes()) == XXH_errorcode::XXH_OK;
			}
			bool Finalize(std::span<std::byte> hash) noexcept override
			{
				auto value = std::invoke(finalizeState, m_State);
				if (sizeof(value) <= hash.size_bytes())
				{
					std::memcpy(hash.data(), &value, sizeof(value));
					return true;
				}
				return false;
			}

			// HashCalculator_xxHash1
			auto Compute(IInputStream& stream, uint64_t seed)
			{
				return IHashCalculator::ComputeDefault<sizeof(THashType) * 8>(*this, stream, seed, {});
			}
	};

	using HashCalculator_xxHash1_32bit = HashCalculator_xxHash1<XXH32_hash_t, XXH32_state_t, XXH32_createState, XXH32_freeState, XXH32_reset, XXH32_update, XXH32_digest>;
	using HashCalculator_xxHash1_64bit = HashCalculator_xxHash1<XXH64_hash_t, XXH64_state_t, XXH64_createState, XXH64_freeState, XXH64_reset, XXH64_update, XXH64_digest>;
}

namespace kxf::Crypto
{
	template
	<
		size_t bitLength,
		auto resetState,
		auto resetStateSeed,
		auto resetStateSecret,
		auto resetStateBoth,
		auto updateState,
		auto finalizeState
	>
	class HashCalculator_xxHash3 final: public IHashCalculator
	{
		private:
			XXH3_state_t* m_State = nullptr;

		public:
			HashCalculator_xxHash3() noexcept
			{
				m_State = XXH3_createState();
			}
			~HashCalculator_xxHash3()
			{
				if (m_State)
				{
					XXH3_freeState(m_State);
				}
			}

		public:
			// IHashCalculator
			bool Initialize(uint64_t seed, const SecretValue& secret) noexcept override
			{
				if (m_State)
				{
					XXH_errorcode result = XXH_errorcode::XXH_ERROR;
					if (seed != 0 && secret)
					{
						result = std::invoke(resetStateBoth, m_State, secret.GetData(), secret.GetSize(), seed);
					}
					else if (secret)
					{
						result = std::invoke(resetStateSecret, m_State, secret.GetData(), secret.GetSize());
					}
					else if (seed != 0)
					{
						result = std::invoke(resetStateSeed, m_State, seed);
					}
					else
					{
						result = std::invoke(resetState, m_State);
					}

					return result == XXH_errorcode::XXH_OK;
				}
				return false;
			}
			bool Update(std::span<const std::byte> input) noexcept override
			{
				return std::invoke(updateState, m_State, input.data(), input.size_bytes()) == XXH_errorcode::XXH_OK;
			}
			bool Finalize(std::span<std::byte> hash) noexcept override
			{
				auto value = std::invoke(finalizeState, m_State);
				if (sizeof(value) <= hash.size_bytes())
				{
					std::memcpy(hash.data(), &value, sizeof(value));
					return true;
				}
				return false;
			}

			// HashCalculator_xxHash3
			auto Compute(IInputStream& stream, uint64_t seed, const SecretValue& secret)
			{
				return IHashCalculator::ComputeDefault<bitLength>(*this, stream, seed, secret);
			}
	};

	using HashCalculator_xxHash3_64bit = HashCalculator_xxHash3<64, XXH3_64bits_reset, XXH3_64bits_reset_withSeed, XXH3_64bits_reset_withSecret, XXH3_64bits_reset_withSecretandSeed, XXH3_64bits_update, XXH3_64bits_digest>;
	using HashCalculator_xxHash3_128bit = HashCalculator_xxHash3<128, XXH3_128bits_reset, XXH3_128bits_reset_withSeed, XXH3_128bits_reset_withSecret, XXH3_128bits_reset_withSecretandSeed, XXH3_128bits_update, XXH3_128bits_digest>;
}
