#pragma once


#include <stdafx.h>

#include <Memory\Viewer.h>
#include <Memory\Allocator.hpp>


namespace fs
{
	namespace Memory
	{
		template<typename T>
		inline Viewer<T>::Viewer()
			: _memoryAllocator{ &fs::Memory::Allocator<T>::getInstance() }
			, _memoryAccesor{ _memoryAllocator }
		{
			__noop;
		}

		template<typename T>
		inline Viewer<T>::Viewer(const T& instance)
			: Viewer()
		{
			_memoryAccesor = _memoryAllocator->allocate();
		
			// instance ��� �� memoryAccessor �� �ִٸ� �� Reference Count �� �����մϴ�!!!
			_memoryAccesor.setMemory(&instance);
		}

		/*
		template<typename T>
		inline RefPtr<T>::RefPtr(T&& instance)
			: Viewer()
		{
			_memoryAccesor = _memoryAllocator->allocate();

			// instance ��� �� memoryAccessor �� �ִٸ� �� Reference Count ������ ���ϱ� ���� �ʼ�!!!
			memcpy(_memoryAccesor.getMemoryXXX(), &instance, sizeof(T));
		}
		*/

		template<typename T>
		inline Viewer<T>::Viewer(const Viewer& rhs)
			: Viewer()
		{
			memcpy(&_memoryAccesor, &rhs._memoryAccesor, sizeof(_memoryAccesor));
		}

		template<typename T>
		inline Viewer<T>::Viewer(Viewer&& rhs) noexcept
			: Viewer()
		{
			_memoryAccesor = std::move(rhs._memoryAccesor);
		}

		template<typename T>
		inline Viewer<T>& Viewer<T>::operator=(const T& rhs)
		{
			_memoryAccesor = _memoryAllocator->allocate();

			// instance ��� �� memoryAccessor �� �ִٸ� �� Reference Count �� �����մϴ�!!!
			_memoryAccesor.setMemory(&rhs);

			return *this;
		}

		template<typename T>
		inline Viewer<T>& Viewer<T>::operator=(const Viewer<T>& rhs)
		{
			if (this != &rhs)
			{
				//_memoryAccesor = rhs._memoryAccesor;
				memcpy(&_memoryAccesor, &rhs._memoryAccesor, sizeof(_memoryAccesor));
			}
			return *this;
		}

		template<typename T>
		inline Viewer<T>& Viewer<T>::operator=(Viewer<T>&& rhs) noexcept
		{
			if (this != &rhs)
			{
				_memoryAccesor = std::move(rhs._memoryAccesor);
			}
			return *this;
		}

		template<typename T>
		inline const T* Viewer<T>::viewData() const noexcept
		{
			return _memoryAccesor.getMemory();
		}

		template<typename T>
		inline const bool Viewer<T>::isAlive() const noexcept
		{
			return _memoryAccesor.isValid();
		}
	}
}
