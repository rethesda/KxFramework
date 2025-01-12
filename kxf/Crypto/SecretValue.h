#pragma once
#include "Common.h"
#include "kxf/Core/IEncodingConverter.h"

namespace kxf
{
	class KXF_API SecretValue final
	{
		public:
			static SecretValue FromString(const String& str)
			{
				auto utf8 = str.ToUTF8();
				return SecretValue(utf8.data(), utf8.size());
			}
			static SecretValue FromVector(std::vector<std::byte> buffer)
			{
				SecretValue value;
				value.m_Storage = std::move(buffer);

				return value;
			}
			static SecretValue FromBytes(std::span<const std::byte> buffer)
			{
				return SecretValue(buffer.data(), buffer.size_bytes());
			}

		private:
			std::vector<std::byte> m_Storage;

		private:
			void AssignBytes(const void* ptr, size_t size)
			{
				m_Storage.resize(size);
				std::memcpy(m_Storage.data(), ptr, size);
			}

		public:
			SecretValue() noexcept = default;
			SecretValue(const void* ptr, size_t size)
			{
				AssignBytes(ptr, size);
			}
			SecretValue(const SecretValue&) = delete;
			SecretValue(SecretValue&& other) noexcept
			{
				*this = std::move(other);
			}
			~SecretValue() noexcept
			{
				Wipe();
			}

		public:
			bool IsEmpty() const
			{
				return m_Storage.empty();
			}
			size_t GetSize() const noexcept
			{
				return m_Storage.size();
			}
			const void* GetData() const noexcept
			{
				return m_Storage.data();
			}

			std::span<std::byte> GetBuffer() noexcept
			{
				return m_Storage;
			}
			std::span<const std::byte> GetBuffer() const noexcept
			{
				return m_Storage;
			}
			
			String ToString(IEncodingConverter& encodingConverter = EncodingConverter_WhateverWorks) const;
			void Wipe() noexcept;

		public:
			SecretValue& operator=(const SecretValue&) = delete;
			SecretValue& operator=(SecretValue&& other) noexcept
			{
				m_Storage = std::move(other.m_Storage);
				other.Wipe();

				return *this;
			}

			bool operator==(const SecretValue& other) const noexcept
			{
				return m_Storage == other.m_Storage;
			}
			bool operator!=(const SecretValue& other) const noexcept
			{
				return m_Storage != other.m_Storage;
			}

			explicit operator bool() const noexcept
			{
				return !IsEmpty();
			}
			bool operator!() const noexcept
			{
				return IsEmpty();
			}
	};
}
