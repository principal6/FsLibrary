﻿#pragma once


#ifndef FS_IREFLECTIVE_H
#define FS_IREFLECTIVE_H


#include <CommonDefinitions.h>


namespace fs
{
	class ReflectionPool;


	struct ReflectionTypeData
	{
		friend ReflectionPool;

	public:
		ReflectionTypeData() : _byteOffset{ 0 }, _typeSize{ 0 }, _hashCode{ 0 }, _isReflective{ false }, _isRegisterDone{ false } {}
		explicit ReflectionTypeData(const std::type_info& type) : ReflectionTypeData()
		{
			_typeName = _fullTypeName = type.name();

			size_t found = _typeName.rfind("::");
			if (std::string::npos != found)
			{
				_typeName = _typeName.substr(found + 2);
			}

			_hashCode = type.hash_code();
		}

	public:
		const bool operator==(const std::type_info& type) const noexcept
		{
			return _hashCode == type.hash_code();
		}

	public:
		size_t							_byteOffset;
		size_t							_typeSize;
		size_t							_hashCode;

	private:
		bool							_isReflective; // 멤버 중엔 Reflective 하지 않은 자료형이 있을 수 있다.
		bool							_isRegisterDone;
		std::string						_fullTypeName;

	public:
		std::string						_typeName;
		std::string						_declarationName;
		std::vector<ReflectionTypeData>	_memberArray;
	};


	class IReflective;
	class ReflectionPool final
	{
		friend IReflective;

	private:
											ReflectionPool() = default;
											~ReflectionPool() = default;

	private:
		static uint32						registerType(const std::type_info& type, const size_t byteOffset, const size_t typeSize, const std::type_info& memberType, const std::string memberName, const size_t memberSize);
		static void							registerTypeDone(const uint32 typeIndex);
		static const ReflectionTypeData&	getTypeData(const uint32 typeIndex);

	private:
		static ReflectionPool&				getInstance();

	private:
		std::vector<ReflectionTypeData>		_typeArray;
	};


#define FS_DECLARE_REFLECTIVE(type, name)			type name; name.setDeclarationName(#name);

#define FS_REFLECTIVE_CTOR(className)				public: className() { registerTypeInfoXXX(); }
#define FS_REFLECTIVE_CTOR_INIT(className, init)	public: className() : init { registerTypeInfoXXX(); }

#define FS_DECLARE_MEMBER(type, name)				public: type name;

#define FS_REGISTER_BEGIN()							protected: virtual void registerTypeInfoXXX() override {
#define FS_REGISTER_MEMBER(name)						__super::registerType(typeid(*this), reinterpret_cast<size_t>(&(reinterpret_cast<decltype(this)>(0))->name), sizeof(*this), typeid(name), #name, sizeof(name));
#define FS_REGISTER_END()							registerTypeDone(); }


	class IReflective abstract
	{
	public:
									IReflective() = default;
		virtual						~IReflective() = default;

	public:
		void						setDeclarationName(const char* const declarationName)
		{
			_declarationName = declarationName;
		}

		const ReflectionTypeData&	getType() const noexcept
		{
			return fs::ReflectionPool::getTypeData(_myTypeIndex);
		}

		const uint32				getMemberCount() const noexcept
		{
			return _memberCount;
		}

		const ReflectionTypeData&	getMemberType(const uint32 memberIndex) const noexcept
		{
			return fs::ReflectionPool::getTypeData(_myTypeIndex)._memberArray[memberIndex];
		}

	protected:
		void						registerType(const std::type_info& type, const size_t byteOffset, const size_t typeSize, const std::type_info& memberType, const std::string memberName, const size_t memberSize)
		{
			_myTypeIndex = fs::ReflectionPool::registerType(type, byteOffset, typeSize, memberType, memberName, memberSize);
			++_memberCount;
		}

		void						registerTypeDone()
		{
			fs::ReflectionPool::registerTypeDone(_myTypeIndex);
		}

	protected:
		virtual void				registerTypeInfoXXX() abstract;

	protected:
		uint32						_myTypeIndex{ 0 };
		uint32						_memberCount{ 0 };
		const char*					_declarationName{ nullptr };
	};
}


#endif // !FS_IREFLECTIVE_H