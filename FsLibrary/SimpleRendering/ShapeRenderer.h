#pragma once


#ifndef FS_SHAPE_RENDERER_H
#define FS_SHAPE_RENDERER_H


#include <CommonDefinitions.h>

#include <SimpleRendering/IRenderer.h>
#include <SimpleRendering/TriangleBuffer.h>

#include <Math/Int2.h>


namespace fs
{
	namespace SimpleRendering
	{
		class ShapeRenderer final : public IRenderer
		{
		public:
			static constexpr float										kRoundnessAbsoluteBase = 2.0f;

		public:
																		ShapeRenderer(fs::SimpleRendering::GraphicDevice* const graphicDevice);
																		~ShapeRenderer() = default;

		public:
			virtual void												initializeShaders() noexcept override final;
			virtual void												flushData() noexcept override final;
			virtual void												render() noexcept final;

		public:
			void														drawRectangle(const fs::Int2& size, const float angle);
			void														drawRoundedRectangle(const fs::Int2& size, const float roundness, const float angle);
			void														drawTaperedRectangle(const fs::Int2& size, const float tapering, const float bias, const float angle);
			void														drawLine(const fs::Int2& p0, const fs::Int2& p1, const float thickness);

		private:
			fs::Float2													normalizePosition(const fs::Float2& position, const fs::Float2& screenSize);
			void														prepareVertexArray(fs::CppHlsl::VS_INPUT_SHAPE& data, const fs::Float2& position, const fs::Float2& halfSize);
			void														prepareIndexArray();

		private:
			SimpleRendering::TriangleBuffer<CppHlsl::VS_INPUT_SHAPE>	_shapeBuffer;
			DxObjectId													_vertexShader;
			DxObjectId													_pixelShader;
		};
	}
}


#endif // !FS_SHAPE_RENDERER_H