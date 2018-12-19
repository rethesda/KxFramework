/*
Copyright � 2018 Kerber. All rights reserved.

You should have received a copy of the GNU LGPL v3
along with KxFramework. If not, see https://www.gnu.org/licenses/lgpl-3.0.html.
*/
#pragma once
#include "KxFramework/KxFramework.h"

class KX_API KxCOMInit
{
	private:
		HRESULT m_Result = 0;
		bool m_IsInit = false;

	public:
		KxCOMInit(DWORD options);
		~KxCOMInit();

	public:
		HRESULT GetResult() const
		{
			return m_Result;
		}
		void Uninitialize();

		operator bool() const
		{
			return m_IsInit;
		}
		bool operator!() const
		{
			return !m_IsInit;
		}
};

//////////////////////////////////////////////////////////////////////////
template<class T> class KxCOMPtr
{
	private:
		T* m_Ptr = nullptr;

	public:
		KxCOMPtr(T* ptr = nullptr)
			:m_Ptr(ptr)
		{
			static_assert(std::is_base_of<IUnknown, T>::value, "class T is not derived from IUnknown");
		}
		~KxCOMPtr()
		{
			if (m_Ptr)
			{
				m_Ptr->Release();
				m_Ptr = nullptr;
			}
		}

	public:
		void Reset(T* newPtr = nullptr)
		{
			if (m_Ptr)
			{
				m_Ptr->Release();
			}
			m_Ptr = newPtr;
		}
		T* Detach()
		{
			T* ptr = m_Ptr;
			m_Ptr = nullptr;
			return ptr;
		}

		T* Get() const
		{
			return m_Ptr;
		}
		void** GetPVoid() const
		{
			return reinterpret_cast<void**>(const_cast<T**>(&m_Ptr));
		}
		
		operator T*()
		{
			return m_Ptr;
		}
		T** operator&()
		{
			return &m_Ptr;
		}

		operator bool() const
		{
			return m_Ptr != nullptr;
		}
		bool operator!() const
		{
			return m_Ptr == nullptr;
		}

		T* operator->() const
		{
			return m_Ptr;
		}
		T* operator->()
		{
			return m_Ptr;
		}

		KxCOMPtr& operator=(const KxCOMPtr&) = delete;
		KxCOMPtr& operator=(T* ptr)
		{
			Reset(ptr);
			return *this;
		}
};
