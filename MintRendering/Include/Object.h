#pragma once


#ifndef MINT_OBJECT_H
#define MINT_OBJECT_H


#include <MintCommon/Include/CommonDefinitions.h>

#include <MintContainer/Include/Vector.h>

#include <MintMath/Include/Float4x4.h>

#include <MintRendering/Include/IObjectComponent.h>


namespace mint
{
    namespace Rendering
    {
        class IObjectComponent;
        class TransformComponent;
        class ObjectPool;
        enum class ObjectComponentType;
        struct Srt;

        enum class ObjectType
        {
            INVALID,
            Object,
            CameraObject
        };


        class Object
        {
            friend ObjectPool;

        private:
                                            Object(const ObjectPool* const objectPool);

        protected:
                                            Object(const ObjectPool* const objectPool, const ObjectType objectType);
            virtual                         ~Object();
        
        public:
            void                            attachComponent(IObjectComponent* const objectComponent);
            void                            detachComponent(IObjectComponent* const objectComponent);
        
        public:
            const ObjectType                getType() const noexcept;
            const bool                      isTypeOf(const ObjectType objectType) const noexcept;
            const uint32                    getComponentCount() const noexcept;
            IObjectComponent*               getComponent(const ObjectComponentType type) const noexcept;

        public:
            void                            setObjectTransform(const mint::Rendering::Srt& srt) noexcept;
            mint::Rendering::Srt&             getObjectTransformSrt() noexcept;
            const mint::Rendering::Srt&       getObjectTransformSrt() const noexcept;
            mint::Float4x4                    getObjectTransformMatrix() const noexcept;

        protected:
            TransformComponent*             getObjectTransformComponent() const noexcept;
            const float                     getDeltaTimeS() const noexcept;

        protected:
            const ObjectPool* const         _objectPool;
            const ObjectType                _objectType;

        protected:
            mint::Vector<IObjectComponent*>   _componentArray;
        };

    }
}


#include <MintRendering/Include/Object.inl>


#endif // !MINT_OBJECT_H
