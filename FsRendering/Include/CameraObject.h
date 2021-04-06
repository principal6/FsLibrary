#pragma once


#ifndef FS_CAMERA_OBJECT_H
#define FS_CAMERA_OBJECT_H


#include <FsRendering/Include/Object.h>


namespace fs
{
    namespace Rendering
    {
        class ObjectPool;


        class CameraObject final : public Object
        {
            friend ObjectPool;

        public:
            enum class MoveDirection
            {
                Forward,
                Backward,
                Leftward,
                Rightward,
                Upward,
                Downward
            };

        private:
            enum class MoveSpeed : uint8
            {
                x0_125,
                x0_25,
                x0_5,
                x1_0,
                x2_0,
                x4_0,
                x8_0,
                x16_0,
                COUNT
            };

            FS_INLINE constexpr const float getMoveSpeedAsFloat(const MoveSpeed moveSpeed)
            {
                static_assert(static_cast<uint8>(MoveSpeed::x0_125) == 0, "Base �� �޶������ϴ�!");
                const uint8 moveSpeedUint8 = static_cast<uint8>(moveSpeed);
                return 0.125f * static_cast<float>(fs::Math::pow2_ui32(moveSpeedUint8));
            }

            FS_INLINE constexpr const MoveSpeed getFasterMoveSpeed(const MoveSpeed moveSpeed)
            {
                const uint8 moveSpeedUint8 = static_cast<uint8>(moveSpeed);
                const uint8 moveSpeedMax = static_cast<uint8>(MoveSpeed::COUNT) - 1;
                if (moveSpeedUint8 < moveSpeedMax)
                {
                    return static_cast<MoveSpeed>(moveSpeedUint8 + 1);
                }
                return moveSpeed;
            }

            FS_INLINE constexpr const MoveSpeed getSlowerMoveSpeed(const MoveSpeed moveSpeed)
            {
                const uint8 moveSpeedUint8 = static_cast<uint8>(moveSpeed);
                if (0 < moveSpeedUint8)
                {
                    return static_cast<MoveSpeed>(moveSpeedUint8 - 1);
                }
                return moveSpeed;
            }

        private:
                                    CameraObject(const ObjectPool* const objectPool);
                                    
        public:
            virtual                 ~CameraObject();
        
        public:
            void                    setPerspectiveFov(const float fov);
            void                    setPerspectiveZRange(const float nearZ, const float farZ);
            void                    setPerspectiveScreenRatio(const float screenRatio);

        private:
            void                    updatePerspectiveMatrix() noexcept;
        
        public:
            void                    move(const MoveDirection moveDirection);
            void                    increaseMoveSpeed() noexcept;
            void                    decreaseMoveSpeed() noexcept;
            void                    setIsBoostMode(const bool isBoostMode) noexcept;

        public:
            void                    rotatePitch(const float angle);
            void                    rotateYaw(const float angle);

        public:
            fs::Float4x4            getViewMatrix() const noexcept;
            const fs::Float4x4&     getProjectionMatrix() const noexcept;
        
        private:
            fs::Float4x4            getRotationMatrix() const noexcept;

        private:
            fs::Float3              _baseUpDirection;
            fs::Float3              _baseForwardDirection;
            fs::Float3              _focusOffset;
            fs::Float4x4            _projectionMatrix;

        private:
            mutable fs::Float3      _forwardDirectionFinal;

        private:
            float                   _fov;
            float                   _nearZ;
            float                   _farZ;
            float                   _screenRatio;

        private:
            float                   _pitch;
            float                   _yaw;
            float                   _roll;
            MoveSpeed               _moveSpeed;
            bool                    _isBoostMode;
            float                   _rotationFactor;
        };
    }
}


#endif // !FS_CAMERA_OBJECT_H
