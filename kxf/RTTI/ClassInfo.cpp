#include "kxf-pch.h"
#include "ClassInfo.h"
#include "kxf/Core/String.h"

namespace
{
	kxf::RTTI::ClassInfo* m_FirstClassInfo = nullptr;

	std::string_view DoGetFullyQualifiedName(std::string_view sourceName, size_t index) noexcept
	{
		constexpr auto npos = std::string_view::npos;

		// Examples:
		//	kxf::RTTI::ClassInfoOf<class kxf::ITimeClock,class kxf::IObject>::ClassInfoOf
		//	kxf::RTTI::ClassInfoOf<class kxf::EventSystem::SignalInvocationEvent<0,void (__cdecl Kortex::IGameInstance::*)(void)>,class kxf::BasicEvent,class kxf::ISignalInvocationEvent>::ClassInfoOf
		//	kxf::RTTI::ClassInfoOf<class Kortex::DefaultApplication,class Kortex::IApplication,class kxf::Application::ICommandLine>::ClassInfoOf

		// Extract the first listed class which is the actual class name, others are base classes
		size_t classBegin = sourceName.find("<class ");
		if (classBegin != npos)
		{
			// Advance for the requested base class name
			for (size_t i = 1; i <= index && classBegin != npos; i++)
			{
				// Offset by 1 to force search for next anchor
				classBegin = sourceName.find(",class ", classBegin + 1);
			}

			if (classBegin != npos)
			{
				// Search for next class in the list or for the end
				size_t classEnd = sourceName.find(",class ", classBegin + 1);
				if (classEnd == npos)
				{
					classEnd = sourceName.rfind(">::ClassInfoOf");
				}

				if (classEnd != npos)
				{
					// Now extract the name
					auto fullyQualifiedName = sourceName.substr(classBegin, classEnd - classBegin);
					fullyQualifiedName.remove_prefix(7);

					return fullyQualifiedName;
				}
			}
		}
		return {};
	}

	enum class NamePart
	{
		ClassName,
		Namespace,
	};
	std::string_view DoGetNamePart(std::string_view fullyQualifiedName, NamePart desiredName) noexcept
	{
		constexpr auto npos = std::string_view::npos;
		if (size_t namespaceIndex = fullyQualifiedName.rfind("::"); namespaceIndex != npos)
		{
			if (desiredName == NamePart::ClassName)
			{
				return fullyQualifiedName.substr(namespaceIndex + 2);
			}
			else if (desiredName == NamePart::Namespace)
			{
				return fullyQualifiedName.substr(0, namespaceIndex);
			}
		}
		else
		{
			if (desiredName == NamePart::ClassName)
			{
				return fullyQualifiedName;
			}
		}
		return {};
	}
}

namespace kxf::RTTI::Private
{
	class BaseClassesEnumerator final
	{
		private:
			const ClassInfo& m_This;
			std::vector<const ClassInfo*> m_SubDirectories;
			std::vector<const ClassInfo*> m_NextSubDirectories;
			bool m_Recurse = false;

			size_t m_Index = 0;
			size_t m_Count = std::numeric_limits<size_t>::max();

		protected:
			CallbackCommand SearchDirectory(CallbackFunction<const ClassInfo&>& callback, const ClassInfo& directory, std::vector<const ClassInfo*>& childDirectories, bool& subTreeDone)
			{
				if (m_Count == std::numeric_limits<size_t>::max())
				{
					m_Count = directory.DoGetBaseClass(nullptr);
				}

				if (m_Index < m_Count)
				{
					const ClassInfo* classInfo = nullptr;
					directory.DoGetBaseClass(&classInfo, m_Index++);

					if (classInfo)
					{
						if (m_Recurse && classInfo->DoGetBaseClass(nullptr) != 0)
						{
							childDirectories.emplace_back(classInfo);
						}
						return callback.Invoke(*classInfo).GetLastCommand();
					}
					else
					{
						// TODO: Investigate missing RTTI class infos. Most likely it's because kxf is compiled
						// as a static library instead of a DLL.

						// Returned class info shouldn't be nullptr as they must always be there but sometimes
						// we still can't find them for some reason. This shouldn't really happen but it happens
						// anyway. Skip such items.

						return CallbackCommand::Discard;
					}
				}
				else
				{
					subTreeDone = true;
					m_Count = std::numeric_limits<size_t>::max();
					m_Index = 0;

					if (m_Recurse)
					{
						return CallbackCommand::Discard;
					}
					return CallbackCommand::Terminate;
				}
			};

		public:
			BaseClassesEnumerator() = default;
			BaseClassesEnumerator(const ClassInfo& thisClassInfo, bool recurse = false)
				:m_This(thisClassInfo), m_Recurse(recurse)
			{
			}

		public:
			CallbackResult<void> Run(CallbackFunction<const ClassInfo&>& callback)
			{
				// Do the current level
				bool subTreeDone = false;
				CallbackCommand command = SearchDirectory(callback, m_This, m_SubDirectories, subTreeDone);

				// Do the nested levels if we're allowed to and there are any
				while (!m_SubDirectories.empty() && !callback.ShouldTerminate() && command != CallbackCommand::Terminate)
				{
					for (auto classInfo: m_SubDirectories)
					{
						command = SearchDirectory(callback, *classInfo, m_NextSubDirectories, subTreeDone);
						if (callback.ShouldTerminate() || command == CallbackCommand::Terminate)
						{
							break;
						}
					}

					m_SubDirectories = std::move(m_NextSubDirectories);
				}
				return callback.Finalize();
			}
	};
}

namespace kxf::RTTI
{
	// ClassInfo
	const ClassInfo* ClassInfo::GetFirstClassInfo() noexcept
	{
		return m_FirstClassInfo;
	}

	void ClassInfo::OnCreate() noexcept
	{
		m_NextClassInfo = m_FirstClassInfo;
		m_FirstClassInfo = this;
	}
	void ClassInfo::OnDestroy() noexcept
	{
		m_FirstClassInfo = m_NextClassInfo;
	}

	std::shared_ptr<IObject> ClassInfo::DoCreateImplementation(const IID& iid) const
	{
		if (m_Traits.Contains(ClassTrait::Interface) && iid)
		{
			std::shared_ptr<IObject> ref;
			EnumDynamicImplementations([&](const ClassInfo& classInfo)
			{
				if (classInfo.GetIID() == iid)
				{
					ref = classInfo.DoCreateObjectInstance();
					return CallbackCommand::Terminate;
				}
				return CallbackCommand::Continue;
			});
			return ref;
		}
		return nullptr;
	}
	std::shared_ptr<IObject> ClassInfo::DoCreateImplementation(const String& fullyQualifiedName) const
	{
		if (m_Traits.Contains(ClassTrait::Interface) && !fullyQualifiedName.IsEmpty())
		{
			std::shared_ptr<IObject> ref;
			EnumDynamicImplementations([&](const ClassInfo& classInfo)
			{
				if (fullyQualifiedName.IsSameAs(classInfo.m_FullyQualifiedName))
				{
					ref = classInfo.DoCreateObjectInstance();
					return CallbackCommand::Terminate;
				}
				return CallbackCommand::Continue;
			});
			return ref;
		}
		return nullptr;
	}
	std::shared_ptr<IObject> ClassInfo::DoCreateAnyImplementation() const
	{
		if (m_Traits.Contains(ClassTrait::Interface))
		{
			std::shared_ptr<IObject> ref;
			EnumDynamicImplementations([&](const ClassInfo& classInfo)
			{
				ref = classInfo.DoCreateObjectInstance();
				return CallbackCommand::Terminate;
			});
			return ref;
		}
		return nullptr;
	}

	// IObject
	RTTI::QueryInfo ClassInfo::DoQueryInterface(const IID& iid) noexcept
	{
		return {};
	}

	// ClassInfo
	std::string_view ClassInfo::ParseToFullyQualifiedName(std::string_view name, size_t index) const noexcept
	{
		return DoGetFullyQualifiedName(name, index);
	}

	String ClassInfo::GetClassName() const
	{
		return DoGetNamePart(m_FullyQualifiedName, NamePart::ClassName);
	}
	kxf::String ClassInfo::GetNamespace() const
	{
		return DoGetNamePart(m_FullyQualifiedName, NamePart::Namespace);
	}
	String ClassInfo::GetFullyQualifiedName() const
	{
		return m_FullyQualifiedName;
	}

	bool ClassInfo::IsNull() const noexcept
	{
		return m_TypeInfo == nullptr;
	}
	bool ClassInfo::IsBaseOf(const ClassInfo& other) const noexcept
	{
		bool result = false;
		other.EnumBaseClasses([&](const ClassInfo& classInfo)
		{
			if (classInfo == *this)
			{
				result = true;
				return CallbackCommand::Terminate;
			}
			return CallbackCommand::Continue;
		});
		return result;
	}
	bool ClassInfo::IsSameAs(const ClassInfo& other) const noexcept
	{
		if (this == &other)
		{
			return true;
		}
		else
		{
			return m_Size == other.m_Size && m_Alignment == other.m_Alignment && m_Traits == other.m_Traits && m_FullyQualifiedName == other.m_FullyQualifiedName;
		}
	}

	CallbackResult<void> ClassInfo::EnumImmediateBaseClasses(CallbackFunction<const ClassInfo&> func) const noexcept
	{
		return Private::BaseClassesEnumerator(*this, false).Run(func);
	}
	CallbackResult<void> ClassInfo::EnumBaseClasses(CallbackFunction<const ClassInfo&> func) const noexcept
	{
		return Private::BaseClassesEnumerator(*this, true).Run(func);
	}
	CallbackResult<void> ClassInfo::EnumDerivedClasses(CallbackFunction<const ClassInfo&> func) const noexcept
	{
		for (auto classInfo = m_FirstClassInfo; classInfo; classInfo = classInfo->m_NextClassInfo)
		{
			if (this->IsBaseOf(*classInfo) && func.Invoke(*classInfo).ShouldTerminate())
			{
				break;
			}
		}
		return func.Finalize();
	}
	CallbackResult<void> ClassInfo::EnumImplementations(CallbackFunction<const ClassInfo&> func) const noexcept
	{
		EnumDerivedClasses([&](const ClassInfo& classInfo)
		{
			auto traits = classInfo.GetTraits();
			if (traits.Contains(ClassTrait::Implementation) && !traits.Contains(ClassTrait::Abstract|ClassTrait::Private))
			{
				return func.Invoke(classInfo).GetLastCommand();
			}
			return CallbackCommand::Discard;
		});
		return func.Finalize();
	}
	CallbackResult<void> ClassInfo::EnumDynamicImplementations(CallbackFunction<const ClassInfo&> func) const noexcept
	{
		EnumDerivedClasses([&](const ClassInfo& classInfo)
		{
			auto traits = classInfo.GetTraits();
			if (traits.Contains(ClassTrait::Dynamic|ClassTrait::Implementation) && !traits.Contains(ClassTrait::Abstract|ClassTrait::Private))
			{
				return func.Invoke(classInfo).GetLastCommand();
			}
			return CallbackCommand::Discard;
		});
		return func.Finalize();
	}
	CallbackResult<void> ClassInfo::EnumDerivedInterfaces(CallbackFunction<const ClassInfo&> func) const noexcept
	{
		EnumDerivedClasses([&](const ClassInfo& classInfo)
		{
			auto traits = classInfo.GetTraits();
			if (traits.Contains(ClassTrait::Interface) && !traits.Contains(ClassTrait::Private))
			{
				return func.Invoke(classInfo).GetLastCommand();
			}
			return CallbackCommand::Discard;
		});
		return func.Finalize();
	}
}
