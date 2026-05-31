#include "kxf-pch.h"
#include "WMI.h"
#include "SafeArray.h"
#include "VariantProperty.h"

#include <Windows.h>
#include <wbemcli.h>
#include "kxf/Win32/LinkLibs-COM.h"

#include "Private/BeginIncludeCOM.h"
namespace
{
	kxf_MakeWinUnicodeCallWrapper(FormatMessage);
}
#include "Private/EndIncludeCOM.h"

namespace
{
	using namespace kxf;

	constexpr long g_ConnectServerFlags = WBEM_FLAG_CONNECT_USE_MAX_WAIT;

	_bstr_t ToBSTR(const String& value)
	{
		return value.wc_str().data();
	}
	String FromBSTR(const _bstr_t& bstr)
	{
		return String(static_cast<const XChar*>(bstr), static_cast<size_t>(bstr.length()));
	}
	String FromBSTR(const BSTR bstr)
	{
		return String(static_cast<const XChar*>(bstr));
	}

	const BSTR StringOrNull(const _bstr_t& value) noexcept
	{
		if (value.length() != 0)
		{
			return static_cast<const BSTR>(value);
		}
		return nullptr;
	}

	COMPtr<IEnumWbemClassObject> GetWbemEnumerator(IWbemServices& service, const String& query)
	{
		COMPtr<IEnumWbemClassObject> wbemEnumerator;
		if (HResult result = service.ExecQuery(ToBSTR("WQL"), ToBSTR(query), WBEM_FLAG_FORWARD_ONLY|WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &wbemEnumerator))
		{
			return wbemEnumerator;
		}
		return nullptr;
	}
}

namespace kxf
{
	bool WMINamespace::Initialize(const kxf::FSPath& wmiNamespace, const UserCredentials* credentials, const Locale& locale)
	{
		if (HResult result = COM::CreateInstance(CLSID_WbemLocator, COMClassContext::InprocServer, &m_Locator))
		{
			auto ns = ToBSTR(wmiNamespace.GetFullPath());
			auto localeName = ToBSTR(locale.GetName());

			if (credentials)
			{
				auto userName = ToBSTR(credentials->GetName());
				if (credentials->HasSecret())
				{
					auto password = ToBSTR(credentials->GetSecret().ToString());
					result = m_Locator->ConnectServer(ns, userName, password, StringOrNull(localeName), g_ConnectServerFlags, nullptr, nullptr, &m_Service);
				}
				else
				{
					result = m_Locator->ConnectServer(ns, userName, nullptr, StringOrNull(localeName), g_ConnectServerFlags, nullptr, nullptr, &m_Service);
				}
			}
			else
			{
				result = m_Locator->ConnectServer(ns, nullptr, nullptr, StringOrNull(localeName), g_ConnectServerFlags, nullptr, nullptr, &m_Service);
			}

			if (result)
			{
				result = ::CoSetProxyBlanket(m_Service.Get(), RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, 0, EOAC_NONE);
			}
			return result.IsSuccess();
		}
		return false;
	}

	WMINamespace::WMINamespace() noexcept = default;
	WMINamespace::WMINamespace(const FSPath& wmiNamespace, const Locale& locale)
	{
		Initialize(wmiNamespace, nullptr, locale);
	}
	WMINamespace::WMINamespace(const FSPath& wmiNamespace, const UserCredentials& credentials, const Locale& locale)
	{
		Initialize(wmiNamespace, &credentials, locale);
	}
	WMINamespace::WMINamespace(const WMINamespace&) noexcept = default;
	WMINamespace::WMINamespace(WMINamespace&&) noexcept = default;
	WMINamespace::~WMINamespace() = default;

	bool WMINamespace::IsNull() const noexcept
	{
		return m_Locator.IsNull() || m_Service.IsNull();
	}

	CallbackResult<void> WMINamespace::EnumClassNames(CallbackFunction<String> func, TimeSpan timeout) const
	{
		const_cast<WMINamespace&>(*this).SelectAll("meta_class", [&](WMIClassObject classObject)
		{
			// Only return the class object if it has its 'dynamic' qualifier set to true
			if (auto qualifierSet = classObject.GetQualifierSet())
			{
				if (qualifierSet.GetValue("Dynamic").QueryAs<bool>() == true)
				{
					String name = classObject.GetClassName();
					if (!name.IsEmpty() && func.Invoke(name).ShouldTerminate())
					{
						return CallbackCommand::Terminate;
					}
				}
			}
			return CallbackCommand::Continue;
		}, timeout);
		return func.Finalize();
	}
	CallbackResult<void> WMINamespace::EnumChildNamespaces(CallbackFunction<String> func, TimeSpan timeout) const
	{
		const_cast<WMINamespace&>(*this).SelectAll("__NAMESPACE", [&](WMIClassObject classObject)
		{
			String name = classObject.GetName();
			if (!name.IsEmpty() && func.Invoke(name).ShouldTerminate())
			{
				return CallbackCommand::Terminate;
			}
			return CallbackCommand::Continue;
		}, timeout);
		return func.Finalize();
	}

	CallbackResult<void> WMINamespace::ExecuteQuery(const kxf::String& query, CallbackFunction<WMIClassObject> func, TimeSpan timeout)
	{
		if (auto wbemEnumerator = GetWbemEnumerator(*m_Service, query))
		{
			while (true)
			{
				ULONG count = 0;
				COMPtr<IWbemClassObject> classObject;
				HResult result = wbemEnumerator->Next([&]() -> long
				{
					if (timeout.IsNegative())
					{
						return WBEM_INFINITE;
					}
					else if (timeout.IsNull())
					{
						return WBEM_NO_WAIT;
					}
					else
					{
						return timeout.GetMilliseconds();
					}
				}(), 1, & classObject, & count);
				if (result && count != 0 && !func.Invoke(std::move(classObject)).ShouldTerminate())
				{
					continue;
				}
				break;
			}
			return func.Finalize();
		}
		return {};
	}
	CallbackResult<void> WMINamespace::SelectAll(const kxf::String& fromLocation, CallbackFunction<WMIClassObject> func, TimeSpan timeout)
	{
		// TODO: Add input sanitization of some kind?
		return ExecuteQuery(Format("SELECT * FROM {}", fromLocation), std::move(func), timeout);
	}

	WMINamespace& WMINamespace::operator=(const WMINamespace&) noexcept = default;
	WMINamespace& WMINamespace::operator=(WMINamespace&& other) noexcept = default;
}

namespace kxf
{
	WMIClassObject::WMIClassObject() noexcept = default;
	WMIClassObject::WMIClassObject(COMPtr<IWbemClassObject> classObject) noexcept
		:m_ClassObject(std::move(classObject))
	{
	}
	WMIClassObject::WMIClassObject(const WMIClassObject&) noexcept = default;
	WMIClassObject::WMIClassObject(WMIClassObject&&) noexcept = default;
	WMIClassObject::~WMIClassObject() = default;

	bool WMIClassObject::IsNull() const noexcept
	{
		return m_ClassObject.IsNull();
	}
	bool WMIClassObject::IsSameAs(const WMIClassObject& other, FlagSet<WMIClassObjectComparisonFlag> flags) const noexcept
	{
		if (this == &other || IsNull() && other.IsNull())
		{
			return true;
		}
		else
		{
			auto MapFlags = [](FlagSet<WMIClassObjectComparisonFlag> flags) constexpr noexcept
			{
				FlagSet<long> nativeFlags;
				nativeFlags.Add(WBEM_FLAG_IGNORE_OBJECT_SOURCE, flags & WMIClassObjectComparisonFlag::IgnoreObjectSource);
				nativeFlags.Add(WBEM_FLAG_IGNORE_DEFAULT_VALUES, flags & WMIClassObjectComparisonFlag::IgnoreDefaultValues);
				nativeFlags.Add(WBEM_FLAG_IGNORE_QUALIFIERS, flags & WMIClassObjectComparisonFlag::IgnoreQualifiers);
				nativeFlags.Add(WBEM_FLAG_IGNORE_FLAVOR, flags & WMIClassObjectComparisonFlag::IgnoreFlavor);
				nativeFlags.Add(WBEM_FLAG_IGNORE_CLASS, flags & WMIClassObjectComparisonFlag::IgnoreClass);
				nativeFlags.Add(WBEM_FLAG_IGNORE_CASE, flags & WMIClassObjectComparisonFlag::IgnoreCase);

				return *nativeFlags;
			};
			return HResult(m_ClassObject->CompareTo(MapFlags(flags), &const_cast<IWbemClassObject&>(*other.m_ClassObject))).IsSuccess();
		}
	}

	String WMIClassObject::GetName() const
	{
		return GetProperty("Name").GetAs<String>();
	}
	String WMIClassObject::GetClassName() const
	{
		return GetProperty("__Class").GetAs<String>();
	}

	CallbackResult<void> WMIClassObject::EnumPropertyNames(CallbackFunction<String> func, FlagSet<WMIClassObjectFlag> flags) const
	{
		auto MapFlags = [](FlagSet<WMIClassObjectFlag> flags) constexpr noexcept
		{
			FlagSet<long> nativeFlags;
			nativeFlags.Add(WBEM_FLAG_LOCAL_ONLY, flags & WMIClassObjectFlag::LocalOnly);
			nativeFlags.Add(WBEM_FLAG_PROPAGATED_ONLY, flags & WMIClassObjectFlag::PropagatedOnly);
			nativeFlags.Add(WBEM_FLAG_SYSTEM_ONLY, flags & WMIClassObjectFlag::SystemOnly);
			nativeFlags.Add(WBEM_FLAG_NONSYSTEM_ONLY, flags & WMIClassObjectFlag::NonSystemOnly);

			return *nativeFlags;
		};

		SafeArray nameArray;
		if (HResult result = m_ClassObject->GetNames(nullptr, MapFlags(flags), nullptr, nameArray.GetAddress()))
		{
			size_t size = nameArray.GetSize();
			auto items = nameArray.GetItems<BSTR>();

			for (size_t i = 0; i < size; i++)
			{
				if (func.Invoke(FromBSTR(items[i])).ShouldTerminate())
				{
					break;
				}
			}
			return func.Finalize();
		}
		return {};
	}
	Any WMIClassObject::GetProperty(const kxf::String& name) const
	{
		VARIANT value;
		if (HResult(m_ClassObject->Get(name.wc_str(), WBEM_FLAG_ALWAYS, &value, nullptr, nullptr)))
		{
			return VariantProperty(value).ToAny();
		}
		return {};
	}

	WMIQualifierSet WMIClassObject::GetQualifierSet() const
	{
		COMPtr<IWbemQualifierSet> qualifierSet;
		if (HResult result = m_ClassObject->GetQualifierSet(&qualifierSet))
		{
			return qualifierSet;
		}
		return {};
	}
	WMIQualifierSet WMIClassObject::GetMethodQualifierSet(const kxf::String& name) const
	{
		COMPtr<IWbemQualifierSet> qualifierSet;
		if (HResult result = m_ClassObject->GetMethodQualifierSet(name.wc_str(), &qualifierSet))
		{
			return qualifierSet;
		}
		return {};
	}
	WMIQualifierSet WMIClassObject::GetPropertyQualifierSet(const kxf::String& name) const
	{
		COMPtr<IWbemQualifierSet> qualifierSet;
		if (HResult result = m_ClassObject->GetPropertyQualifierSet(name.wc_str(), &qualifierSet))
		{
			return qualifierSet;
		}
		return {};
	}

	WMIClassObject& WMIClassObject::operator=(const WMIClassObject&) noexcept = default;
	WMIClassObject& WMIClassObject::operator=(WMIClassObject&& other) noexcept = default;
}
namespace kxf
{
	WMIQualifierSet::WMIQualifierSet() noexcept = default;
	WMIQualifierSet::WMIQualifierSet(COMPtr<IWbemQualifierSet> qualifierSet) noexcept
		:m_QualifierSet(std::move(qualifierSet))
	{
	}
	WMIQualifierSet::WMIQualifierSet(const WMIQualifierSet&) noexcept = default;
	WMIQualifierSet::WMIQualifierSet(WMIQualifierSet&&) noexcept = default;
	WMIQualifierSet::~WMIQualifierSet() = default;

	bool WMIQualifierSet::IsNull() const noexcept
	{
		return m_QualifierSet.IsNull();
	}

	CallbackResult<void> WMIQualifierSet::EnumQualifiers(CallbackFunction<String, Any> func, FlagSet<WMIClassObjectFlag> flags) const
	{
		auto MapFlags = [](FlagSet<WMIClassObjectFlag> flags) constexpr noexcept
		{
			FlagSet<long> nativeFlags;
			nativeFlags.Add(WBEM_FLAG_LOCAL_ONLY, flags & WMIClassObjectFlag::LocalOnly);
			nativeFlags.Add(WBEM_FLAG_PROPAGATED_ONLY, flags & WMIClassObjectFlag::PropagatedOnly);

			return *nativeFlags;
		};

		if (HResult result = m_QualifierSet->BeginEnumeration(MapFlags(flags)))
		{
			while (result.IsSuccess())
			{
				BSTRPtr name;
				VARIANT value;
				if (result = m_QualifierSet->Next(0, &name, &value, nullptr))
				{
					if (func.Invoke(name.Get(), VariantProperty(value).ToAny()).ShouldTerminate())
					{
						break;
					}
				}
			}
			m_QualifierSet->EndEnumeration();

			return func.Finalize();
		}
		return {};
	}
	CallbackResult<void> WMIQualifierSet::EnumNames(CallbackFunction<String> func, FlagSet<WMIClassObjectFlag> flags) const
	{
		auto MapFlags = [](FlagSet<WMIClassObjectFlag> flags) constexpr noexcept
		{
			FlagSet<long> nativeFlags;
			nativeFlags.Add(WBEM_FLAG_LOCAL_ONLY, flags & WMIClassObjectFlag::LocalOnly);
			nativeFlags.Add(WBEM_FLAG_PROPAGATED_ONLY, flags & WMIClassObjectFlag::PropagatedOnly);

			return *nativeFlags;
		};
		SafeArray nameArray;
		if (HResult result = m_QualifierSet->GetNames(MapFlags(flags), nameArray.GetAddress()))
		{
			size_t size = nameArray.GetSize();
			auto items = nameArray.GetItems<BSTR>();

			for (size_t i = 0; i < size; i++)
			{
				if (func.Invoke(FromBSTR(items[i])).ShouldTerminate())
				{
					break;
				}
			}
			return func.Finalize();
		}
		return {};
	}
	Any WMIQualifierSet::GetValue(const kxf::String& name) const
	{
		VARIANT value;
		if (HResult result = m_QualifierSet->Get(name.wc_str(), 0, &value, nullptr))
		{
			return VariantProperty(value).ToAny();
		}
		return {};
	}

	WMIQualifierSet& WMIQualifierSet::operator=(const WMIQualifierSet&) noexcept = default;
	WMIQualifierSet& WMIQualifierSet::operator=(WMIQualifierSet&& other) noexcept = default;
}
