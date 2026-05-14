#pragma once
#include "Common.h"
#include "String.h"
#include "kxf/Core/CallbackFunction.h"
#include "kxf/Utility/ScopeGuard.h"
#include "Private/XDocument.h"
class IInputStream;
class IOutputStream;

namespace kxf::XDocument
{
	enum class AsCDATA
	{
		Auto = -1,
		Always = 1,
		Never = 0,
	};
}

namespace kxf
{
	class KXF_API IXDocument: public RTTI::Interface<IXDocument>
	{
		kxf_RTTI_DeclareIID(IXDocument, {0x2382c534, 0x417e, 0x41b0, {0xa7, 0xdd, 0x94, 0x18, 0x6c, 0x25, 0xc4, 0xcc}});

		public:
			using AsCDATA = XDocument::AsCDATA;
			static constexpr size_t npos = std::numeric_limits<size_t>::max();

		public:
			virtual ~IXDocument() = default;

		public:
			virtual bool IsNull() const = 0;
			virtual UniChar GetXPathSeparator(UniChar* indexSeparator = nullptr) const
			{
				if (indexSeparator)
				{
					*indexSeparator = ':';
				}
				return '/';
			}
			virtual String GetDocumentMeta() const = 0;

			virtual bool LoadDocument(IInputStream& stream) = 0;
			virtual bool SaveDocument(IOutputStream& stream) const = 0;

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

	class KXF_API IXDocumentNode
	{
		public:
			using AsCDATA = XDocument::AsCDATA;
			static constexpr size_t npos = std::numeric_limits<size_t>::max();

			template<std::derived_from<IXDocumentNode> TNode>
			static String BacktrackXPath(const IXDocument& document, const TNode& startAt)
			{
				UniChar indexSeparator;
				UniChar separator = document.GetXPathSeparator(&indexSeparator);
				bool isFirst = true;

				String result;
				for (TNode node = startAt; node; node = node.GetParent())
				{
					size_t index = node.GetIndexWithinParent();
					if (index != 0)
					{
						result.FormatAt(0, "{}{}{}", node.GetName(), indexSeparator.GetAs<XChar>(), index);
					}
					else
					{
						result.Prepend(node.GetName());
					}

					if (!isFirst)
					{
						result.Prepend(separator);
					}
					isFirst = false;
				}
				return result;
			}

		public:
			virtual ~IXDocumentNode() = default;

		public:
			virtual bool IsNull() const = 0;
			virtual String GetXPath() const = 0;

			virtual String GetName() const = 0;
			virtual size_t GetIndexWithinParent() const = 0;
			virtual size_t GetRelativeIndexWithinParent() const = 0;

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

namespace kxf::XDocument
{
	template<class TDerived>
	class DefaultConverter
	{
		private:
			TDerived& GetDerived() noexcept
			{
				return static_cast<TDerived&>(*this);
			}
			const TDerived& GetDerived() const noexcept
			{
				return static_cast<const TDerived&>(*this);
			}

		public:
			// Read
			std::optional<bool> XDocument_ConvertToBool(const String& value) const
			{
				return value.ParseBoolean();
			}
			std::optional<void*> XDocument_ConvertToPointer(const String& value) const
			{
				return value.ParsePointer();
			}

			template<class T>
			requires(std::is_integral_v<T> || std::is_enum_v<T>)
			std::optional<T> XDocument_ConvertToInt(const String& value, int base = 10) const
			{
				return value.ParseInteger<T>(base);
			}

			template<class T>
			requires(std::is_floating_point_v<T>)
			std::optional<T> XDocument_ConvertToFloat(const String& value) const
			{
				return value.ParseFloatingPoint<T>();
			}

		public:
			// Write
			String XDocument_ConvertFromBool(bool value) const
			{
				return String::FromBoolean(value);
			}
			String XDocument_ConvertFromPointer(void* value) const
			{
				return String::FromPointer(value);
			}

			template<class T>
			requires(std::is_integral_v<T> || std::is_enum_v<T>)
			String XDocument_ConvertFromInt(T value, int base = 10) const
			{
				return String::FromInteger(value, base);
			}

			template<class T>
			requires(std::is_floating_point_v<T>)
			String XDocument_ConvertFromFloat(T value, int precision = -1) const
			{
				return String::FromFloatingPoint(value, precision);
			}
	};

	// Requires:
	// - std::optional<String> XDocument_QueryValue() const
	//
	// Can customize:
	// - std::optional<int^> XDocument_ConvertToInt(const String&) const
	// - std::optional<float^> XDocument_ConvertToFloat(const String&) const
	// - std::optional<bool> XDocument_ConvertToBool(const String&) const
	// - std::optional<void> XDocument_ConvertToPointer(const String&) const
	template<class TDerived>
	class ROValue
	{
		private:
			TDerived& GetDerived() noexcept
			{
				return static_cast<TDerived&>(*this);
			}
			const TDerived& GetDerived() const noexcept
			{
				return static_cast<const TDerived&>(*this);
			}

		public:
			template<class T = String>
			requires(std::is_same_v<T, String>)
			std::optional<String> QueryValue() const
			{
				return GetDerived().XDocument_QueryValue();
			}

			template<class T>
			requires(std::is_same_v<T, bool>)
			std::optional<bool> QueryValue() const
			{
				if (auto value = GetDerived().XDocument_QueryValue())
				{
					return GetDerived().XDocument_ConvertToBool(*value);
				}
				return {};
			}

			template<class T>
			requires(std::is_same_v<T, void*>)
			std::optional<void*> QueryValue() const
			{
				if (auto value = GetDerived().XDocument_QueryValue())
				{
					return GetDerived().XDocument_ConvertToPointer(*value);
				}
				return {};
			}

			template<class T>
			requires((std::is_integral_v<T> || std::is_enum_v<T>) && !std::is_same_v<T, bool>)
			std::optional<T> QueryValue(int base = 10) const
			{
				if (auto value = GetDerived().XDocument_QueryValue())
				{
					return GetDerived().XDocument_ConvertToInt<T>(*value, base);
				}
				return {};
			}

			template<class T>
			requires(std::is_floating_point_v<T>)
			std::optional<T> QueryValue() const
			{
				if (auto value = GetDerived().XDocument_QueryValue())
				{
					return GetDerived().XDocument_ConvertToFloat<T>(*value);
				}
				return {};
			}

			// Query the data or return the default-constructed value
			template<class T = String, class... Args>
			T GetValue(Args&&... arg) const
			{
				return QueryValue<T>(std::forward<Args>(arg)...).value_or(T{});
			}
	};

	// Requires
	// - See ROValue
	// - bool XDocument_WriteValue(const String&, AsCDATA)
	//
	// Can customize (all of ROValue and):
	// - See ROValue
	// - String XDocument_ConvertFromInt(int^) const
	// - String XDocument_ConvertFromFloat(float^, int) const
	// - String XDocument_ConvertFromBool(bool) const
	// - String XDocument_ConvertFromPointer(void*) const
	template<class TDerived>
	class RWValue: public ROValue<TDerived>
	{
		private:
			TDerived& GetDerived() noexcept
			{
				return static_cast<TDerived&>(*this);
			}
			const TDerived& GetDerived() const noexcept
			{
				return static_cast<const TDerived&>(*this);
			}

		public:
			bool SetValue(const String& value, AsCDATA asCDATA = AsCDATA::Auto)
			{
				return GetDerived().XDocument_WriteValue(value, asCDATA);
			}
			bool SetValue(const char* value, AsCDATA asCDATA = AsCDATA::Auto)
			{
				return GetDerived().XDocument_WriteValue(value, asCDATA);
			}
			bool SetValue(const wchar_t* value, AsCDATA asCDATA = AsCDATA::Auto)
			{
				return GetDerived().XDocument_WriteValue(value, asCDATA);
			}
			bool SetValue(bool value)
			{
				return GetDerived().XDocument_WriteValue(GetDerived().XDocument_ConvertFromBool(value), AsCDATA::Never);
			}
			bool SetValue(void* value)
			{
				return GetDerived().XDocument_WriteValue(GetDerived().XDocument_ConvertFromPointer(value), AsCDATA::Never);
			}
			bool SetValue(std::nullptr_t)
			{
				return GetDerived().XDocument_WriteValue(GetDerived().XDocument_ConvertFromPointer(nullptr), AsCDATA::Never);
			}

			template<class T>
			requires((std::is_integral_v<T> || std::is_enum_v<T>) && !std::is_same_v<T, bool>)
			bool SetValue(T value, int base = 10)
			{
				return GetDerived().XDocument_WriteValue(GetDerived().XDocument_ConvertFromInt<T>(value, base), AsCDATA::Never);
			}

			template<class T>
			requires(std::is_floating_point_v<T>)
			bool SetValue(T value, int precision = -1)
			{
				return GetDerived().XDocument_WriteValue(GetDerived().XDocument_ConvertFromFloat<T>(value, precision), AsCDATA::Never);
			}
	};

	// Requires:
	// - std::optional<String> XDocument_QueryAttribute(const String&) const
	//
	// Can customize:
	// - std::optional<int^> XDocument_ConvertToInt(const String&) const
	// - std::optional<float^> XDocument_ConvertToFloat(const String&) const
	// - std::optional<bool> XDocument_ConvertToBool(const String&) const
	// - std::optional<void> XDocument_ConvertToPointer(const String&) const
	template<class TDerived>
	class ROAttribute
	{
		private:
			TDerived& GetDerived() noexcept
			{
				return static_cast<TDerived&>(*this);
			}
			const TDerived& GetDerived() const noexcept
			{
				return static_cast<const TDerived&>(*this);
			}

		public:
			template<class T = String>
			requires(std::is_same_v<T, String>)
			std::optional<T> QueryAttribute(const String& name) const
			{
				return GetDerived().XDocument_QueryAttribute(name);
			}

			template<class T>
			requires(std::is_same_v<T, bool>)
			std::optional<T> QueryAttribute(const String& name) const
			{
				if (auto value = GetDerived().XDocument_QueryAttribute(name))
				{
					return GetDerived().XDocument_ConvertToBool(*value);
				}
				return {};
			}

			template<class T>
			requires(std::is_same_v<T, void*>)
			std::optional<T> QueryAttribute(const String& name) const
			{
				if (auto value = GetDerived().XDocument_QueryAttribute(name))
				{
					return GetDerived().XDocument_ConvertToPointer(*value);
				}
				return {};
			}

			template<class T>
			requires((std::is_integral_v<T> || std::is_enum_v<T>) && !std::is_same_v<T, bool>)
			std::optional<T> QueryAttribute(const String& name, int base = 10) const
			{
				if (auto value = GetDerived().XDocument_QueryAttribute(name))
				{
					return GetDerived().XDocument_ConvertToInt<T>(*value, base);
				}
				return {};
			}

			template<class T>
			requires(std::is_floating_point_v<T>)
			std::optional<T> QueryAttribute(const String& name) const
			{
				if (auto value = GetDerived().XDocument_QueryAttribute(name))
				{
					return GetDerived().XDocument_ConvertToFloat<T>(*value);
				}
				return {};
			}

			// Query the data or return the default-constructed value
			template<class T = String, class... Args>
			T GetAttribute(const String& name, Args&&... arg) const
			{
				return QueryAttribute<T>(name, std::forward<Args>(arg)...).value_or(T{});
			}
	};

	// Requires:
	// - See ROAttribute
	// - bool XDocument_WriteAttribute(const String&, const String&, AsCDATA)
	//
	// Can customize:
	// - See ROAttribute
	// - String XDocument_ConvertFromInt(int^) const
	// - String XDocument_ConvertFromFloat(float^, int) const
	// - String XDocument_ConvertFromBool(bool) const
	// - String XDocument_ConvertFromPointer(void*) const
	template<class TDerived>
	class RWAttribute: public ROAttribute<TDerived>
	{
		private:
			TDerived& GetDerived() noexcept
			{
				return static_cast<TDerived&>(*this);
			}
			const TDerived& GetDerived() const noexcept
			{
				return static_cast<const TDerived&>(*this);
			}

		public:
			bool SetAttribute(const String& name, const String& value, AsCDATA asCDATA = AsCDATA::Auto)
			{
				return GetDerived().XDocument_WriteAttribute(name, value, asCDATA);
			}
			bool SetAttribute(const String& name, const char* value, AsCDATA asCDATA = AsCDATA::Auto)
			{
				return GetDerived().XDocument_WriteAttribute(name, value, asCDATA);
			}
			bool SetAttribute(const String& name, const wchar_t* value, AsCDATA asCDATA = AsCDATA::Auto)
			{
				return GetDerived().XDocument_WriteAttribute(name, value, asCDATA);
			}
			bool SetAttribute(const String& name, bool value)
			{
				return GetDerived().XDocument_WriteAttribute(name, GetDerived().XDocument_ConvertFromBool(value), AsCDATA::Never);
			}
			bool SetAttribute(const String& name, void* value)
			{
				return GetDerived().XDocument_WriteAttribute(name, GetDerived().XDocument_ConvertFromPointer(value), AsCDATA::Never);
			}
			bool SetAttribute(const String& name, std::nullptr_t)
			{
				return GetDerived().XDocument_WriteAttribute(name, GetDerived().XDocument_ConvertFromPointer(nullptr), AsCDATA::Never);
			}

			template<class T>
			requires((std::is_integral_v<T> || std::is_enum_v<T>) && !std::is_same_v<T, bool>)
			bool SetAttribute(const String& name, T value, int base = 10)
			{
				return GetDerived().XDocument_WriteAttribute(name, GetDerived().XDocument_ConvertFromInt<T>(value, base), AsCDATA::Never);
			}

			template<class T>
			requires(std::is_floating_point_v<T>)
			bool SetAttribute(const String& name, T value, int precision = -1)
			{
				return GetDerived().XDocument_WriteAttribute(name, GetDerived().XDocument_ConvertFromFloat<T>(value, precision), AsCDATA::Never);
			}
	};
}
