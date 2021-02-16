#include <stdafx.h>
#include <FsRenderingBase/Include/ShapeFontRendererContext.h>

#include <FsRenderingBase/Include/GraphicDevice.h>


namespace fs
{
	namespace SimpleRendering
	{
		ShapeFontRendererContext::ShapeFontRendererContext(fs::SimpleRendering::GraphicDevice* const graphicDevice)
			: ShapeRendererContext(graphicDevice)
			, _fontRendererContext{ graphicDevice, _triangleRenderer }
		{
			__noop;
		}

		ShapeFontRendererContext::~ShapeFontRendererContext()
		{
			__noop;
		}

		void ShapeFontRendererContext::initializeShaders() noexcept
		{
			fs::SimpleRendering::DxShaderPool& shaderPool = _graphicDevice->getShaderPool();

			{
				static constexpr const char kShaderString[]
				{
					R"(
					#include <ShaderStructDefinitions>
					#include <ShaderConstantBuffers>
					#include <ShaderStructuredBufferDefinitions>
					
					StructuredBuffer<SB_Transform> sbTransform : register(t0);
					
					VS_OUTPUT_SHAPE main_shape(VS_INPUT_SHAPE input)
					{
						const uint shapeInfo = asuint(input._info.y);
						const uint shapeType = (shapeInfo >> 30) & 3;
						const uint shapeIndex = shapeInfo & 0x3FFFFFFF;
						
						float4 transformedPosition = float4(input._position.xyz, 1.0);
						transformedPosition = mul(transformedPosition, sbTransform[shapeIndex]._transformMatrix);
						
						VS_OUTPUT_SHAPE result = (VS_OUTPUT_SHAPE)0;
						result._position		= float4(mul(transformedPosition, _cbProjectionMatrix).xyz, 1.0);
						result._color			= input._color;
						result._texCoord		= input._texCoord;
						result._info			= input._info;
						result._info.x			= (float)shapeType;
						result._viewportIndex	= (uint)input._info.x;
						
						return result;
					}
					)"
				};
				const Language::CppHlslTypeInfo& typeInfo = _graphicDevice->getCppHlslStructs().getTypeInfo(typeid(fs::CppHlsl::VS_INPUT_SHAPE));
				_vertexShaderId = shaderPool.pushVertexShader("ShapeFontRendererVS", kShaderString, "main_shape", &typeInfo);
			}

			{
				static constexpr const char kShaderString[]
				{
					R"(
					#include <ShaderStructDefinitions>
					
					[maxvertexcount(3)]
					void main_shape(triangle VS_OUTPUT_SHAPE input[3], inout TriangleStream<VS_OUTPUT_SHAPE> OutputStream)
					{
						for (int i = 0; i < 3; ++i)
						{
							OutputStream.Append(input[i]);
						}
						OutputStream.RestartStrip();
					}
					)"
				};
				_geometryShaderId = shaderPool.pushNonVertexShader("ShapeFontRendererGS", kShaderString, "main_shape", DxShaderType::GeometryShader);
			}

			{
				static constexpr const char kShaderString[]
				{
					R"(
					#include <ShaderStructDefinitions>
					#include <ShaderConstantBuffers>

					static const float kDeltaDoublePixel = _cbProjectionMatrix[0][0];
					static const float kDeltaPixel = kDeltaDoublePixel * 0.5;
					static const float kDeltaHalfPixel = kDeltaPixel * 0.5;
					
					sampler					sampler0;
					Texture2D<float>		texture0;
					
					float4 main_shape(VS_OUTPUT_SHAPE input) : SV_Target
					{
						// =====
						// Font rendering
						// =====
						if (1.0 == input._info.z)
						{
							const float sampled = texture0.Sample(sampler0, input._texCoord.xy);
							float4 sampledColor = float4(input._color.xyz * ((0.0 < sampled) ? 1.0 : 0.0), sampled * input._color.a);
						
							if (input._info.y == 1.0)
							{
								const float2 rbCoord = input._texCoord - float2(ddx(input._texCoord.x), ddy(input._texCoord.y));
								const float rbSampled = texture0.Sample(sampler0, rbCoord);
								if (0.0 < rbSampled)
								{
									const float3 rbColor = lerp(sampledColor.xyz * 0.25 * max(rbSampled, 0.25), sampledColor.xyz, sampled);
									return float4(rbColor, saturate(sampled + rbSampled));
								}
							}
							return sampledColor;
						}
						
						// =====
						// Shape rendering
						// =====
						const float u = input._texCoord.x;
						const float v = input._texCoord.y;
						const float scale = input._texCoord.w;
						const float flipped = input._texCoord.z;
						
						float signedDistance = 0.0;
						if (1.0 == input._info.x)
						{
							// Solid triangle
							return input._color;
						}
						else if (2.0 == input._info.x)
						{
							// Circular section
							signedDistance = flipped * (1.0 -  sqrt(u * u + v * v));
						}
						else if (3.0 == input._info.x)
						{
							// Circle
							signedDistance = flipped * (1.0 - sqrt(input._texCoord.x * input._texCoord.x + input._texCoord.y * input._texCoord.y));
						}
						if (0.0 == input._info.x)
						{
							// Quadratic Bezier
							signedDistance = -(u * u - v);
						}
						
						// Apply scale to the signed distance for more consistent anti-aliasing
						if (0.0 < scale)
						{
							signedDistance *= (scale * kDeltaPixel);
						}
						
						const float alpha = (kDeltaHalfPixel < signedDistance) ? 1.0 : 1.0 - saturate(abs(signedDistance - kDeltaHalfPixel) / kDeltaPixel);
						return float4(input._color.xyz, input._color.w * alpha);
					}
					)"
				};
				_pixelShaderId = shaderPool.pushNonVertexShader("ShapeFontRendererPS", kShaderString, "main_shape", DxShaderType::PixelShader);
			}
		}

		void ShapeFontRendererContext::flushData() noexcept
		{
			_triangleRenderer->flush();

			flushShapeTransform();

			// @�߿�
			// Identity SB_Transform for font rendering!!
			{
				fs::CppHlsl::SB_Transform sbTransformIdentity;
				sbTransformIdentity._transformMatrix.setIdentity();
				_sbTransformData.emplace_back(sbTransformIdentity);
			}
		}

		const bool ShapeFontRendererContext::hasData() const noexcept
		{
			return _triangleRenderer->isRenderable();
		}

		void ShapeFontRendererContext::render() noexcept
		{
			_graphicDevice->getResourcePool().bindToShader(_fontRendererContext.getFontTextureId(), fs::SimpleRendering::DxShaderType::PixelShader, 0);

			__super::render();
		}

		void ShapeFontRendererContext::setViewportIndex(const uint32 viewportIndex) noexcept
		{
			__super::setViewportIndex(viewportIndex);

			_fontRendererContext.setViewportIndex(viewportIndex);
		}

		const bool ShapeFontRendererContext::initializeFontData(const FontRendererContext::FontData& fontData) noexcept
		{
			return _fontRendererContext.loadFontData(fontData);
		}

		const FontRendererContext::FontData& ShapeFontRendererContext::getFontData() const noexcept
		{
			return _fontRendererContext.getFontData();
		}

		void ShapeFontRendererContext::drawDynamicText(const wchar_t* const wideText, const fs::Float4& position, const TextRenderDirectionHorz directionHorz, const TextRenderDirectionVert directionVert, const float scale, const bool drawShade)
		{
			_fontRendererContext.drawDynamicText(wideText, position, directionHorz, directionVert, scale, drawShade);
		}

		void ShapeFontRendererContext::drawDynamicText(const wchar_t* const wideText, const uint32 textLength, const fs::Float4& position, const TextRenderDirectionHorz directionHorz, const TextRenderDirectionVert directionVert, const float scale, const bool drawShade)
		{
			_fontRendererContext.drawDynamicText(wideText, textLength, position, directionHorz, directionVert, scale, drawShade);
		}

		const float ShapeFontRendererContext::calculateTextWidth(const wchar_t* const wideText, const uint32 textLength) const noexcept
		{
			return _fontRendererContext.calculateTextWidth(wideText, textLength);
		}

		void ShapeFontRendererContext::setTextColor(const fs::SimpleRendering::Color& textColor) noexcept
		{
			_fontRendererContext.setColor(textColor);
		}

	}
}
