#pragma once
#include "kxf/Utility/ConstEvalString.h"

#include "kxf/Win32/UndefMacros.h"
#include <wx/module.h>
#include <wx/object.h>
#include <wx/rtti.h>
#include <wx/xti.h>
#include "kxf/Win32/UndefMacros.h"

#define	KxWxRTTI_ImplementClassDynamic(Name, CClassName, Base1CClassName)														\
wxClassInfo CClassName::ms_classInfo(wxT(#Name), &Base1CClassName::ms_classInfo, nullptr, (int)sizeof(CClassName), nullptr);			\
wxClassInfo* CClassName::GetClassInfo() const																					\
{																																\
	return &CClassName::ms_classInfo;																							\
}

#define	KxWxRTTI_ImplementClassDynamic2(Name, CClassName, Base1CClassName, Base2CClassName)															\
wxClassInfo CClassName::ms_classInfo(wxT(#Name), &Base1CClassName::ms_classInfo, &Base2CClassName::ms_classInfo, (int)sizeof(CClassName), nullptr);	\
wxClassInfo* CClassName::GetClassInfo() const																										\
{																																					\
	return &CClassName::ms_classInfo;																												\
}

namespace kxf::wxWidgets
{
	template<class TDerived, class TBaseClass, Utility::ConstEvalStringW className>
	class RTTI_DynamicObject: public TBaseClass
	{
		public:
			static wxObject* wxCreateObject()
			{
				return new TDerived();
			};
			inline static wxClassInfo ms_classInfo = wxClassInfo(className.data(), &TBaseClass::ms_classInfo, nullptr, static_cast<int>(sizeof(TBaseClass)), &RTTI_DynamicObject::wxCreateObject);

		public:
			// wxObject
			wxClassInfo* GetClassInfo() const override
			{
				return &ms_classInfo;
			}
	};
}
