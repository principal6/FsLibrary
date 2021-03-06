#pragma once


#include <stdafx.h>
#include <MintRendering/Include/ObjectPool.h>

#include <MintContainer/Include/Vector.hpp>

#include <MintRenderingBase/Include/GraphicDevice.h>
#include <MintRenderingBase/Include/LowLevelRenderer.hpp>

#include <MintRendering/Include/MeshComponent.h>
#include <MintRendering/Include/CameraObject.h>
#include <MintRendering/Include/DeltaTimer.h>


namespace mint
{
    namespace Rendering
    {
        inline ObjectPool::ObjectPool()
            : _deltaTimer{ &mint::Rendering::DeltaTimer::getDeltaTimer() }
        {
            __noop;
        }
        
        inline ObjectPool::~ObjectPool()
        {
            destroyObjects();
        }

        MINT_INLINE mint::Rendering::Object* ObjectPool::createObject()
        {
            return createObjectInternalXXX(MINT_NEW(mint::Rendering::Object, this));
        }

        MINT_INLINE mint::Rendering::CameraObject* ObjectPool::createCameraObject()
        {
            return static_cast<mint::Rendering::CameraObject*>(createObjectInternalXXX(MINT_NEW(mint::Rendering::CameraObject, this)));
        }

        MINT_INLINE void ObjectPool::destroyObjects()
        {
            const uint32 objectCount = getObjectCount();
            for (uint32 objectIndex = 0; objectIndex < objectCount; ++objectIndex)
            {
                if (_objectArray[objectIndex] != nullptr)
                {
                    destroyObjectComponents(*_objectArray[objectIndex]);

                    MINT_DELETE(_objectArray[objectIndex]);
                }
            }
            _objectArray.clear();
        }

        MINT_INLINE mint::Rendering::Object* ObjectPool::createObjectInternalXXX(mint::Rendering::Object* const object)
        {
            _objectArray.push_back(object);
            object->attachComponent(createTransformComponent()); // 모든 Object는 TransformComponent 를 필수로 가집니다.
            return object;
        }

        MINT_INLINE mint::Rendering::TransformComponent* ObjectPool::createTransformComponent()
        {
            return MINT_NEW(mint::Rendering::TransformComponent);
        }

        MINT_INLINE mint::Rendering::MeshComponent* ObjectPool::createMeshComponent()
        {
            mint::Rendering::MeshComponent* result = MINT_NEW(mint::Rendering::MeshComponent);
            _meshComponentArray.push_back(std::move(result));
            return _meshComponentArray.back();
        }

        MINT_INLINE void ObjectPool::destroyObjectComponents(Object& object)
        {
            const uint32 componentCount = static_cast<uint32>(object._componentArray.size());
            for (uint32 componentIndex = 0; componentIndex < componentCount; ++componentIndex)
            {
                IObjectComponent*& component = object._componentArray[componentIndex];
                if (component != nullptr)
                {
                    const ObjectComponentType componentType = component->getType();
                    if (componentType == ObjectComponentType::MeshComponent)
                    {
                        deregisterMeshComponent(static_cast<mint::Rendering::MeshComponent*>(component));
                    }

                    MINT_DELETE(component);
                }
            }
        }

        MINT_INLINE void ObjectPool::registerMeshComponent(mint::Rendering::MeshComponent* const meshComponent)
        {
            if (meshComponent == nullptr)
            {
                return;
            }

            const uint32 meshComponentCount = static_cast<uint32>(_meshComponentArray.size());
            for (uint32 meshComponentIndex = 0; meshComponentIndex < meshComponentCount; ++meshComponentIndex)
            {
                if (_meshComponentArray[meshComponentIndex]->getId() == meshComponent->getId())
                {
                    return;
                }
            }

            _meshComponentArray.push_back(meshComponent);
        }

        MINT_INLINE void ObjectPool::deregisterMeshComponent(mint::Rendering::MeshComponent* const meshComponent)
        {
            if (meshComponent == nullptr)
            {
                return;
            }

            int32 foundIndex = -1;
            const int32 meshComponentCount = static_cast<int32>(_meshComponentArray.size());
            for (int32 meshComponentIndex = 0; meshComponentIndex < meshComponentCount; ++meshComponentIndex)
            {
                if (_meshComponentArray[meshComponentIndex]->getId() == meshComponent->getId())
                {
                    foundIndex = meshComponentIndex;
                    break;
                }
            }

            if (0 <= foundIndex)
            {
                if (foundIndex < meshComponentCount)
                {
                    std::swap(_meshComponentArray[foundIndex], _meshComponentArray.back());
                }
                _meshComponentArray.pop_back();
            }
        }

        MINT_INLINE void ObjectPool::computeDeltaTime() const noexcept
        {
            _deltaTimer->computeDeltaTimeS();
        }

        MINT_INLINE void ObjectPool::updateScreenSize(const mint::Float2& screenSize)
        {
            const float screenRatio = (screenSize._x / screenSize._y);
            const uint32 objectCount = _objectArray.size();
            for (uint32 objectIndex = 0; objectIndex < objectCount; ++objectIndex)
            {
                mint::Rendering::Object*& object = _objectArray[objectIndex];
                if (object->isTypeOf(ObjectType::CameraObject) == true)
                {
                    CameraObject* const cameraObject = static_cast<CameraObject*>(object);
                    cameraObject->setPerspectiveScreenRatio(screenRatio);
                }
            }
        }

        MINT_INLINE const mint::Vector<mint::Rendering::MeshComponent*>& ObjectPool::getMeshComponents() const noexcept
        {
            return _meshComponentArray;
        }
        
        MINT_INLINE const uint32 ObjectPool::getObjectCount() const noexcept
        {
            return static_cast<uint32>(_objectArray.size());
        }

        MINT_INLINE const DeltaTimer* ObjectPool::getDeltaTimerXXX() const noexcept
        {
            return _deltaTimer;
        }
    }
}
