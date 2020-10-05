#pragma once


#ifndef FS_DYNAMIC_STRING_H
#define FS_DYNAMIC_STRING_H


#include <CommonDefinitions.h>
#include <Memory\MemoryAllocator.h>


namespace fs
{
	class DynamicStringA
	{
		static constexpr uint32		kMinCapacity{ 16 };

	public:
									DynamicStringA();
		explicit					DynamicStringA(const uint32 capacity);
									DynamicStringA(const char* const rawString);
									DynamicStringA(const char* const rawString, const uint32 rawStringLength);
									DynamicStringA(const DynamicStringA& rhs);
									DynamicStringA(DynamicStringA&& rhs) noexcept = default;
									~DynamicStringA();

	public:
		DynamicStringA&				operator=(const char* const rawString);
		DynamicStringA&				operator=(const DynamicStringA& rhs);
		DynamicStringA&				operator=(DynamicStringA&& rhs) noexcept = default;

	public:
		DynamicStringA&				operator+=(const char* const rawString) noexcept;
		DynamicStringA&				operator+=(const DynamicStringA& rhs) noexcept;

	public:
		const bool					operator==(const char* const rawString) const noexcept;
		const bool					operator==(const DynamicStringA& rhs) const noexcept;
		const bool					operator!=(const char* const rawString) const noexcept;
		const bool					operator!=(const DynamicStringA& rhs) const noexcept;

	public:
		void						clear();
		void						assign(const char* const rawString);
		void						assign(const char* const rawString, const uint32 rawStringLength);
		void						assign(const DynamicStringA& rhs);
		void						append(const char* const rawString);
		void						append(const DynamicStringA& rhs);
		DynamicStringA				substr(const uint32 offset, const uint32 count = kStringNPos) const noexcept;
		void						setChar(const uint32 at, const char ch);

	public:
		const bool					empty() const noexcept;
		const uint32				length() const noexcept;
		const char* const			c_str() const noexcept;
		const char					getChar(const uint32 at) const noexcept;

	public:
		const uint32				find(const char* const rawString, const uint32 offset = kStringNPos) const noexcept;
		const uint32				rfind(const char* const rawString, const uint32 offset = kStringNPos) const noexcept;
		const bool					compare(const char* const rawString) const noexcept;
		const bool					compare(const DynamicStringA& rhs) const noexcept;
		const uint64				hash() const noexcept;

	private:
		void						setMemoryInternal(const char* const rawString, const uint32 rawStringLength, const uint32 offset, const bool isOneChar = false);

	private:
		static MemoryAllocator		_memoryAllocator;

	private:
		MemoryAccessor				_memoryAccessor;
		uint32						_length;
#if defined FS_DEBUG
		const char*					_debugRawString;
#endif
		mutable uint64				_cachedHash;
	};
}


#endif // !FS_DYNAMIC_STRING_H